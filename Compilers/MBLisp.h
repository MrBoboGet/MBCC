#pragma once
#include "ParserIR.h"
#include "CLikeCompiler.h"

namespace MBCC
{
    class LispParser : public ParserCompiler
    {
        bool IsPolyMorphic(TypeInfo Type)
        {
            return !(Type & ~TypeFlags::Base) && m_Grammar->DepInfo.HasChildren(Type);
        }
        MBCCDefinitions const* m_Grammar = nullptr;
        MBUtility::MBOctetOutputStream* m_SourceStream;

        std::string p_BuiltinToLisp(TypeInfo Type)
        {
            std::string ReturnValue;
            if(Type == TypeFlags::String)
            {
                return "\"\"";
            }
            else if(Type == TypeFlags::Int)
            {
                return "0";
            }
            else if(Type == TypeFlags::Raw)
            {
                return "null";
            }
            else if(Type == TypeFlags::Bool)
            {
                return "false";
            }
            else if(Type == TypeFlags::TokenPos)
            {
                return "TokenPos";
            }
            else if(Type == TypeFlags::TokenPos)
            {
                return "Token";
            }
            return ReturnValue;
        }
        std::string GetTypeAssignment(TypeInfo Type)
        {
            std::string ReturnValue;
            bool IsList = (Type & TypeFlags::List) != 0;
            Type = Type & (~TypeFlags::List);
            if(IsList)
            {
                return "(list)";
            }
            if(Builtin(Type))
            {
                ReturnValue = p_BuiltinToLisp(Type);
            }
            else
            {
                ReturnValue = "(" + m_Grammar->Structs[Type].Name+")"; 
            }
            return(ReturnValue);
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
        //static std::string p_ColumnToBoolArray(MBMath::MBDynamicMatrix<bool> const& Matrix, int k);
        static void p_WriteLOOKTable(MBUtility::MBOctetOutputStream& OutStream,LOOKInfo const& Look);
        static void p_WriteStructs(MBUtility::MBOctetOutputStream& OutStream,MBCCDefinitions const& Grammar);
    public:      
        virtual void WriteParser(MBCCDefinitions const& Grammar,LookType const& TotalProductions,std::string const& OutputBase);
        void WriteParser(MBCCDefinitions const& Grammar,LookType const& TotalProductions,MBUtility::MBOctetOutputStream& OutStream);



        bool operator()(Function const& Func)
        {
            *m_SourceStream<<"(defun "<<GetFuncName(Func.NonTerminal,Func.ProductionIndex,Func.Direct)<<" ";
            if(!Func.Direct)
            {
                *m_SourceStream<<"(ReturnValue tokenizer)";
            }
            else
            {
                *m_SourceStream<<"(tokenizer)";
            }
            *m_SourceStream<<"\t\n";
            for(auto const& Content : Func.Content)
            {
                Traverse(*this,Content);
            }
            *m_SourceStream<<"\n\v)\n";
            return false;
        }

        void operator()(Expr_Bool const& Expr)
        {
            *m_SourceStream << (Expr.Value ? "true" : "false");
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
            *m_SourceStream<<":value "<<"(peek tokenizer "<<std::to_string(Expr.PeekIndex)<<")";
        }
        void operator()(Expr_PeekType const& Expr)
        {
            *m_SourceStream<<":type "<<"(peek tokenizer "<<std::to_string(Expr.PeekIndex)<<")";
        }
        void operator()(Expr_PeekPosition const& Expr)
        {
            *m_SourceStream<<":position "<<"(peek tokenizer "<<std::to_string(Expr.PeekIndex)<<")";
        }
        bool operator()(Expr_And const& Expr)
        {
            if(Expr.Negated)
            {
                *m_SourceStream<<"(not ";
            }
            *m_SourceStream<<"(&& ";
            for(auto const& SubExpr : Expr.Arguments)
            {
                Traverse(*this,SubExpr);
            }
            *m_SourceStream<<")";
            if(Expr.Negated)
            {
                *m_SourceStream<<")";
            }
            return false;
        }
        bool operator()(Expr_Equality const& Expr)
        {
            if(Expr.Negated)
            {
                *m_SourceStream<<"(not ";
            }
            *m_SourceStream<<"(eq ";
            Traverse(*this,*Expr.Lhs);
            *m_SourceStream<<" ";
            Traverse(*this,*Expr.Rhs);
            *m_SourceStream<<")";
            if(Expr.Negated)
            {
                *m_SourceStream<<")";   
            }
            return false;
        }
        bool operator()(Expr_Convert const& Expr)
        {
            assert(Expr.SuppliedValue == TypeFlags::String);
            assert(!(Expr.SuppliedValue & TypeFlags::List));

            if(Expr.TargetValue == TypeFlags::Int)
            {
                *m_SourceStream<<"(int ";
                Traverse(*this,*Expr.ValueToConvert);
                *m_SourceStream<<")";
            }
            else if(Expr.TargetValue == TypeFlags::Bool)
            {
                *m_SourceStream << "(eq ";
                Traverse(*this,*Expr.ValueToConvert);
                *m_SourceStream<<" \"true\")";
            }
            else
            {
                assert(false && "Lisp Parser doesn't cover all cases");   
            }
            return false;
        }
        bool operator()(Expr_LOOKValue const& Expr)
        {
            *m_SourceStream<<"(. LOOKTable";
            assert(Expr.Indexes.size() != 0);
            for(int i = 0; i < Expr.Indexes.size();i++)
            {
                *m_SourceStream<<" ";
                Traverse(*this,Expr.Indexes[i]);
            }
            *m_SourceStream<<")";
            return false;
        }
        bool operator()(Expr_GetVar const& Expr)
        {
            assert(Expr.FieldTypes.size() == Expr.Fields.size());
            if(Expr.Fields.size() == 1)
            {
                *m_SourceStream<<Expr.Fields[0];
            }
            else
            {
                *m_SourceStream<<"(.";
                bool First = true;
                for(auto const& Field : Expr.Fields)
                {
                    *m_SourceStream<<" ";
                    if(!First)
                    {
                        *m_SourceStream<<"'";
                    }
                    First = false;
                    *m_SourceStream<<Field;
                }
                *m_SourceStream<<")";
            }
            return false;
        }
        bool operator()(Expr_ParseDirect const& Expr)
        {
            assert(Expr.FieldTypes.size() == Expr.SubFields.size());
            if(Expr.SubFields.size() > 0)
            {
                *m_SourceStream<<"(. ";
            }
            *m_SourceStream<<"(";
            *m_SourceStream<<GetFuncName(Expr.NonTerminal,Expr.ProductionIndex,true);
            *m_SourceStream<<" tokenizer)";
            if(Expr.SubFields.size() > 0)
            {
                for(auto const& Field : Expr.SubFields)
                {
                    *m_SourceStream<<" '"<<Field;
                }   
                *m_SourceStream<<")";
            }
            return false;
        }
        bool operator()(Expr_GetBack const& Expr)
        {
            *m_SourceStream<<"(back ";
            Traverse(*this,Expr.ListVariable);
            *m_SourceStream<<")";
            return false;
        }
        bool operator()(Expr_DefaultConstruct const& Expr)
        {
            *m_SourceStream<<GetTypeAssignment(Expr.ObjectType);
            return false;
        }


        //statements
        void WriteIf(Statement_If const& Statement,bool IsSubIf)
        {
            if(!IsSubIf)
            {
                *m_SourceStream<<"(";
            }
            if(!Statement.Condition.IsType<std::monostate>())
            {
                *m_SourceStream<<"if ";
                Traverse(*this,Statement.Condition);
                *m_SourceStream<<"\t\n";
            }
            for(auto const& SubStatement : Statement.Content)
            {
                Traverse(*this,SubStatement);
            }
            *m_SourceStream<<"\n\v";
            if(!IsSubIf)
            {
                *m_SourceStream<<")";
            }
            *m_SourceStream<<"\n";
        }
        bool operator()(Statement_If const& Statement)
        {
            WriteIf(Statement,false);
            return false;
        }
        bool operator()(Statement_IfChain const& Statement)
        {
            *m_SourceStream<<"(";
            for(int i = 0; i < Statement.Alternatives.size();i++)
            {
                if(i != 0)
                {
                    *m_SourceStream<<"else ";   
                }
                WriteIf(Statement.Alternatives[i],true);
            }
            *m_SourceStream<<"\v)";
            return false;
        }
        bool operator()(Statement_While const& Statement)
        {
            *m_SourceStream<<"(while ";
            Traverse(*this,Statement.Condition);
            *m_SourceStream<<"\t\n";
            for(auto const& SubStatement : Statement.Content)
            {
                Traverse(*this,SubStatement);
            }
            *m_SourceStream<<"\n\v)\n";
            return false;
        }
        bool operator()(Statement_DoWhile const& Statement)
        {
            *m_SourceStream<<"(dowhile ";
            Traverse(*this,Statement.Condition);
            *m_SourceStream<<"\t\n";
            for(auto const& SubStatement : Statement.Content)
            {
                Traverse(*this,SubStatement);
            }
            *m_SourceStream<<"\n\v)\n";
            return false;
        }
        bool operator()(Statement_AssignVar const& Statement)
        {
            *m_SourceStream <<"(setl ";
            (*this)(Statement.Variable);
            *m_SourceStream << " "; 
            Traverse(*this,Statement.Value);
            *m_SourceStream<<")\n";
            return false;
        }
        bool operator()(Statement_AddList const& Statement)
        {
            *m_SourceStream << "(append ";
            (*this)(Statement.Variable);
            *m_SourceStream<<" ";
            Traverse(*this,Statement.Value);
            *m_SourceStream << ")\n"; 
            return false;
        }
        bool operator()(Statement_Return const& Statement)
        {
            *m_SourceStream << "(return "<<Statement.Variable<<")\n"; 
            return false;
        }
        bool operator()(Statement_DeclareVar const& Statement)
        {
            *m_SourceStream << "(setl "<<Statement.Name<< " "<<GetTypeAssignment(Statement.VarType)<<")\n";
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
                "(error (+ \"Error parsing "<<Statement.NonTerminalName<<" at position \" " 
                <<" (str :position (peek tokenizer 0)) \": expected "<<Statement.ExpectedName<<"\"))\n";
            return false;
        }
        bool operator()(Statement_PopToken const& Statement)
        {
            *m_SourceStream<<"(consume-token tokenizer)\n";
            return false;
        }
        bool operator()(Statement_FillVar const& Statement)
        {
            *m_SourceStream<<"(";
            bool Direct = IsPolyMorphic(Statement.VarType);
            if(Direct)
            {
                *m_SourceStream<<"set ";
                Traverse(*this,Statement.ValueToFill);
                *m_SourceStream<<"(";
                *m_SourceStream<<GetFuncName(Statement.NonTerminal,Statement.ProductionIndex,Direct)
                    <<" tokenizer))\n";
            }
            else
            {
                *m_SourceStream<<GetFuncName(Statement.NonTerminal,Statement.ProductionIndex,false)
                    <<" ";
                Traverse(*this,Statement.ValueToFill);
                *m_SourceStream<<" tokenizer)\n";
            }
            return false;
        }
    };
}
