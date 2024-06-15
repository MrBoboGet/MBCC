#pragma once
#include "ParserIR.h"

#include "CLikeCompiler.h"
#include "Cpp.h"
namespace MBCC
{
    class CppIRParser : public ParserCompiler
    {
        MBCCDefinitions const* m_Grammar = nullptr;
        std::unique_ptr<MBUtility::MBOctetOutputStream> m_SourceStream;

        std::string GetTypeString(TypeInfo Type)
        {
            return CPPParserGenerator::GetString(*m_Grammar,Type);   
        }
        bool IsPolyMorphic(TypeInfo Type)
        {
            return !(Type & ~TypeFlags::Base) && m_Grammar->DepInfo.HasChildren(Type);
        }
        std::string GetFuncName(NonTerminalIndex NonTermIndex,int Production,bool Direct)
        {
            std::string ReturnValue = Direct ? "Parse" : "Fill";
            ReturnValue += m_Grammar->NonTerminals[NonTermIndex].Name;
            if(Production != -1)
            {
                ReturnValue += "_"+std::to_string(Production);
            }
            return ReturnValue;
        }
        static std::string p_ColumnToBoolArray(MBMath::MBDynamicMatrix<bool> const& Matrix, int k);
        static void p_WriteLOOKTable(MBUtility::MBOctetOutputStream& OutStream,LookType const& Look);
    public:      
        virtual void WriteParser(MBCCDefinitions const& Grammar,LookType const& TotalProductions,std::string const& OutputBase);



        bool operator()(Function const& Func)
        {
            std::string ReturnType;
            if(Func.ReturnType != -1 && Func.Direct)
            {
                ReturnType = GetTypeString(Func.ReturnType);
            }
            else
            {
                ReturnType = "void ";   
            }
            *m_SourceStream<<ReturnType<<" "<<GetFuncName(Func.NonTerminal,Func.ProductionIndex,Func.Direct);
            if(!Func.Direct)
            {
                *m_SourceStream<<"("<<GetTypeString(Func.ReturnType)<<"& ReturnValue, MBCC::Tokenizer& Tokenizer)";
            }
            else
            {
                *m_SourceStream<<"(MBCC::Tokenizer& Tokenizer)";
            }
            *m_SourceStream<<"\n{\n";
            for(auto const& Content : Func.Content)
            {
                Traverse(*this,Content);
            }
            *m_SourceStream<<"\n}\n";
            return false;
        }

        void operator()(Expr_Bool const& Expr)
        {
            *m_SourceStream<< (Expr.Value ? "true" : "false");
        }
        void operator()(Expr_Integer const& Expr)
        {
            *m_SourceStream<< std::to_string(Expr.Value);
        }
        void operator()(Expr_String const& Expr)
        {
            *m_SourceStream<<CLikeParser::LiteralEscapeString(Expr.Value);
        }
        void operator()(Expr_PeekValue const& Expr)
        {
            *m_SourceStream<<"Tokenizer.Peek("<<std::to_string(Expr.PeekIndex)<<").Value";
        }
        void operator()(Expr_PeekType const& Expr)
        {
            *m_SourceStream<<"Tokenizer.Peek("<<std::to_string(Expr.PeekIndex)<<").Type";
        }
        void operator()(Expr_PeekPosition const& Expr)
        {
            *m_SourceStream<<"Tokenizer.Peek("<<std::to_string(Expr.PeekIndex)<<").Position";
        }
        bool operator()(Expr_And const& Expr)
        {
            if(Expr.Negated)
            {
                *m_SourceStream<<"!(";
            }
            for(int i = 0; i < Expr.Arguments.size();i++)
            {
                auto const& SubExpr = Expr.Arguments[i];
                Traverse(*this,SubExpr);
                if(i + 1 < Expr.Arguments.size())
                {
                    *m_SourceStream<<" && ";
                }
            }
            if(Expr.Negated)
            {
                *m_SourceStream<<")";
            }
            return false;
        }
        bool operator()(Expr_Equality const& Expr)
        {
            Traverse(*this,*Expr.Lhs);
            *m_SourceStream << (Expr.Negated ? "!=" : "==");
            Traverse(*this,*Expr.Rhs);
            return false;
        }
        bool operator()(Expr_Convert const& Expr)
        {
            assert(Expr.SuppliedValue == TypeFlags::String);
            assert(!(Expr.SuppliedValue & TypeFlags::List));

            if(Expr.TargetValue == TypeFlags::Int)
            {
                *m_SourceStream<<"std::stoi(";
                Traverse(*this,*Expr.ValueToConvert);
                *m_SourceStream<<")";
            }
            else if(Expr.TargetValue == TypeFlags::Bool)
            {
                Traverse(*this,*Expr.ValueToConvert);
                *m_SourceStream<<" == \"true\"";
            }
            else
            {
                assert(false && "Cpp Parser doesn't cover all cases");   
            }
            return false;
        }
        bool operator()(Expr_LOOKValue const& Expr)
        {
            *m_SourceStream<<"LOOKTable[";
            assert(Expr.Indexes.size() != 0);
            for(int i = 0; i < Expr.Indexes.size();i++)
            {
                Traverse(*this,Expr.Indexes[i]);
                *m_SourceStream<<"]";
                if(i + 1  < Expr.Indexes.size())
                {
                    *m_SourceStream<<"[";
                }
            }
            return false;
        }
        bool operator()(Expr_GetVar const& Expr)
        {
            assert(Expr.FieldTypes.size() == Expr.Fields.size());
            for(int i = 0; i < Expr.FieldTypes.size();i++)
            {
                *m_SourceStream<<Expr.Fields[i];
                TypeInfo CurrentType = Expr.FieldTypes[i];
                if(i + 1 < Expr.FieldTypes.size() && !(CurrentType & (~TypeFlags::Base)))
                {
                    if(m_Grammar->DepInfo.HasChildren(CurrentType))
                    {
                        *m_SourceStream<<".GetBase()";
                    }
                }
                if(i + 1 < Expr.FieldTypes.size())
                {
                    *m_SourceStream << ".";
                }
            }
            return false;
        }
        bool operator()(Expr_ParseDirect const& Expr)
        {
            assert(Expr.FieldTypes.size() == Expr.SubFields.size());

            *m_SourceStream<<GetFuncName(Expr.NonTerminal,Expr.ProductionIndex,true);
            *m_SourceStream<<"(Tokenizer)";
            if(Expr.FieldTypes.size() > 0)
            {
                *m_SourceStream<<".";   
            }
            for(int i = 0; i < Expr.FieldTypes.size();i++)
            {
                *m_SourceStream<<Expr.SubFields[i];
                TypeInfo CurrentType = Expr.FieldTypes[i];
                if(i + 1 < Expr.FieldTypes.size() && !(CurrentType & (~TypeFlags::Base)))
                {
                    if(m_Grammar->DepInfo.HasChildren(CurrentType))
                    {
                        *m_SourceStream<<".GetBase()";
                    }
                }
                if(i + 1 < Expr.FieldTypes.size())
                {
                    *m_SourceStream << ".";
                }
            }
            return false;
        }
        bool operator()(Expr_GetBack const& Expr)
        {
            Traverse(*this,Expr.ListVariable);
            *m_SourceStream<<".back()";
            return false;
        }
        bool operator()(Expr_DefaultConstruct const& Expr)
        {
            *m_SourceStream<<GetTypeString(Expr.ObjectType)<<"()";
            return false;
        }


        //statements
        bool operator()(Statement_If const& Statement)
        {
            if(!Statement.Condition.IsType<std::monostate>())
            {
                *m_SourceStream<<"if(";
                Traverse(*this,Statement.Condition);
                *m_SourceStream<<")";
            }
            *m_SourceStream<<"\n{\n";;
            for(auto const& SubStatement : Statement.Content)
            {
                Traverse(*this,SubStatement);
            }
            *m_SourceStream<<"\n}\n";
            return false;
        }
        bool operator()(Statement_IfChain const& Statement)
        {
            for(int i = 0; i < Statement.Alternatives.size();i++)
            {
                if(i != 0)
                {
                    *m_SourceStream<<"else ";   
                }
                (*this)(Statement.Alternatives[i]);
            }
            return false;
        }
        bool operator()(Statement_While const& Statement)
        {
            *m_SourceStream<<"while(";
            Traverse(*this,Statement.Condition);
            *m_SourceStream<<")\n{\n";
            for(auto const& SubStatement : Statement.Content)
            {
                Traverse(*this,SubStatement);
            }
            *m_SourceStream<<"\n}\n";
            return false;
        }
        bool operator()(Statement_DoWhile const& Statement)
        {
            *m_SourceStream<<"do\n{\n";
            for(auto const& SubStatement : Statement.Content)
            {
                Traverse(*this,SubStatement);
            }
            *m_SourceStream<<"\n} while(";
            Traverse(*this,Statement.Condition);
            *m_SourceStream<<");\n";
            return false;
        }
        bool operator()(Statement_AssignVar const& Statement)
        {
            (*this)(Statement.Variable);
            *m_SourceStream << " = "; 
            Traverse(*this,Statement.Value);
            *m_SourceStream<<";\n";
            return false;
        }
        bool operator()(Statement_AddList const& Statement)
        {
            (*this)(Statement.Variable);
            *m_SourceStream << ".push_back("; 
            Traverse(*this,Statement.Value);
            *m_SourceStream << ");\n"; 
            return false;
        }
        bool operator()(Statement_Return const& Statement)
        {
            *m_SourceStream << "return "<<Statement.Variable; 
            *m_SourceStream << ";\n"; 
            return false;
        }
        bool operator()(Statement_DeclareVar const& Statement)
        {
            *m_SourceStream << GetTypeString(Statement.VarType)<<" ";
            *m_SourceStream << Statement.Name<<";\n";
            return false;
        }
        bool operator()(Statement_Expr const& Statement)
        {
            Traverse(*this,Statement.Expr);
            return false;
        }
        bool operator()(Statement_Exception const& Statement)
        {
            *m_SourceStream<<
                "throw MBCC::ParsingException(Tokenizer.Peek().Position,"<<
                CLikeParser::LiteralEscapeString(Statement.NonTerminalName)<<","<<
                CLikeParser::LiteralEscapeString(Statement.ExpectedName)<<");\n";
            return false;
        }
        bool operator()(Statement_PopToken const& Statement)
        {
            *m_SourceStream<<"Tokenizer.ConsumeToken();\n";
            return false;
        }
        bool operator()(Statement_FillVar const& Statement)
        {
            *m_SourceStream<<GetFuncName(Statement.NonTerminal,Statement.ProductionIndex,false)
                <<"(";
            Traverse(*this,Statement.ValueToFill);
            if(IsPolyMorphic(Statement.VarType) && Statement.FillType != Statement.VarType)
            {
                *m_SourceStream<<".GetOrAssign<"<<GetTypeString(Statement.FillType)<<">()";
            }
            *m_SourceStream<<",Tokenizer);\n";
            return false;
        }
    };
};
