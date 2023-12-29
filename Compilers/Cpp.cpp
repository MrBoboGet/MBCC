#include "Cpp.h"

#include <MBUnicode/MBUnicode.h>
namespace MBCC
{
    std::string h_BuiltinToCPP(TypeInfo Info)
    {
        std::string ReturnValue;       
        if(Info & TypeFlags::Token)
        {
            ReturnValue += "token";   
        }
        else if(Info & TypeFlags::String)
        {
            ReturnValue += "std::string";
        }
        else if(Info & TypeFlags::TokenPos)
        {
            ReturnValue += "MBCC::TokenPosition";
        }
        else if(Info & TypeFlags::Bool)
        {
            ReturnValue += "bool";
        }
        else if(Info & TypeFlags::Int)
        {
            ReturnValue += "int";
        }
        return(ReturnValue);
    }
    std::string h_ColumnToBoolArray(MBMath::MBDynamicMatrix<bool> const& Matrix, int k)
    {
        std::string ReturnValue = "{";       
        for(int i = 0; i < Matrix.NumberOfRows();i++)
        {
            if(Matrix(i,k))
            {
                ReturnValue += "true";
            }   
            else
            {
                ReturnValue += "false";   
            }
            ReturnValue += ",";
        } 
        ReturnValue += "}";
        return(ReturnValue);
    }
    int h_UpdateInheritanceOrder(int CurrentOffset,StructIndex CurrentStruct,DependancyInfo const& DepInfo,std::vector<int>& OutBegin,std::vector<int>& OutEnd)
    {
        int OriginalOffset = CurrentOffset;
        OutBegin[CurrentStruct] = CurrentOffset;
        CurrentOffset++;
        for(auto Child : DepInfo.ChildrenMap[CurrentStruct])
        {
            CurrentOffset += h_UpdateInheritanceOrder(CurrentOffset,Child,DepInfo,OutBegin,OutEnd);
        }
        int ReturnValue = CurrentOffset-OriginalOffset;
        OutEnd[CurrentStruct] = CurrentOffset;
        return(ReturnValue);
    }
    std::string h_BuiltinToCpp(std::string const& TypeToVerify)
    {
        std::string ReturnValue = TypeToVerify;
        if(TypeToVerify == "string")
        {
            ReturnValue = "std::string";
        }
        else if(TypeToVerify == "tokenPos")
        {
            ReturnValue = "MBCC::TokenPosition";   
        }
        return(ReturnValue);
    }
    void h_WriteStructures(MBCCDefinitions const& Grammar,DependancyInfo const& DepInfo,MBUtility::MBOctetOutputStream& HeaderOut)
    {
        std::vector<StructIndex> AbstractStructs;
        //One of baseses
        for(StructIndex CurrentIndex : DepInfo.StructureDependancyOrder)
        {
            StructDefinition const& CurrentStruct = Grammar.Structs[CurrentIndex];
            std::string StructName = CurrentStruct.Name;
            if(DepInfo.ChildrenMap[CurrentIndex].size() > 0)
            {
                StructName += "_Base";
                AbstractStructs.push_back(CurrentIndex);
            }
            HeaderOut << "class "<<StructName << " : ";
            if(CurrentStruct.ParentStruct != "")
            {
                HeaderOut<<"public "<<CurrentStruct.ParentStruct;
                if(DepInfo.ChildrenMap[Grammar.NameToStruct.at(CurrentStruct.ParentStruct)].size() > 0)
                {
                    HeaderOut<<"_Base";   
                }
            }
            else
            {
                HeaderOut<<  " public MBCC::AST_Base";
            }
            HeaderOut<<"\n{\n";
            HeaderOut<<"public:\n";
            for(auto const& Member : CurrentStruct.MemberVariables)
            {
                if(Member.IsType<StructMemberVariable_Int>())
                {
                    HeaderOut << "int ";     
                }
                else if(Member.IsType<StructMemberVariable_String>())
                {
                    HeaderOut << "std::string ";   
                }
                else if(Member.IsType<StructMemberVariable_Struct>())
                {
                    HeaderOut << Member.GetType<StructMemberVariable_Struct>().StructType<<" ";
                }
                else if(Member.IsType<StructMemberVariable_Bool>())
                {
                    HeaderOut << "bool ";
                }
                else if(Member.IsType<StructMemberVariable_tokenPosition>())
                {
                    HeaderOut << "MBCC::TokenPosition ";
                }
                else if(Member.IsType<StructMemberVariable_List>())
                {
                    HeaderOut <<"std::vector<";
                    if(TypeIsBuiltin(Member.GetType<StructMemberVariable_List>().ListType))
                    {
                        HeaderOut<<h_BuiltinToCpp(Member.GetType<StructMemberVariable_List>().ListType);
                    }
                    else
                    {
                        HeaderOut<<Member.GetType<StructMemberVariable_List>().ListType;
                    }
                    HeaderOut<<"> ";
                }
                else if(Member.IsType<StructMemberVariable_Raw>())
                {
                    HeaderOut << Member.GetType<StructMemberVariable_Raw>().RawMemberType<<" ";
                }
                HeaderOut<<Member.GetName();
                if(Member.GetDefaultValue().Value != "")
                {
                    HeaderOut<<" = "<<Member.GetDefaultValue().Value; 
                }
                HeaderOut <<";\n";
            }         
            HeaderOut<<"std::unique_ptr<AST_Base> Copy() const override{return(MBCC::CopyAST(*this));}\n";
            HeaderOut<<"\n};\n";
            //If abstract class, write the container class
            if(DepInfo.ChildrenMap[CurrentIndex].size() > 0)
            {
                auto const& ContainerName = Grammar.Structs[CurrentIndex].Name;
                std::string InheritName = "MBCC::PolyBase<"+StructName+">";
                HeaderOut<<"class "<<Grammar.Structs[CurrentIndex].Name<< " : public "<< InheritName<<"\n{\n";
                HeaderOut<<"public:\n";
                HeaderOut<<"typedef "<<InheritName<<" Base;\n";
                HeaderOut<<"using Base::Base;\n";
                if(Grammar.Structs[CurrentIndex].ParentStruct != "")
                {
                    auto const& ParentName = Grammar.Structs[CurrentIndex].ParentStruct;
                    //having a parent always means that that parent is polymorphic
                    HeaderOut<<"operator "<<ParentName<<"()&&\n{\n";
                    HeaderOut<<"return("<<ParentName<<"(std::move(m_Data),m_TypeID));\n}\n";
                }
                HeaderOut<<"\n};\n";
            }
        }
        //Write GetTypeID
        std::vector<int> BeginList = std::vector<int>(Grammar.Structs.size());
        std::vector<int> EndList = std::vector<int>(Grammar.Structs.size());
        int CurrentOffset = 0;
        for(int CurrentStruct = 0; CurrentStruct < Grammar.Structs.size();CurrentStruct++)
        {
            if(Grammar.Structs[CurrentStruct].ParentStruct.empty())
            {
                CurrentOffset += h_UpdateInheritanceOrder(CurrentOffset,CurrentStruct,DepInfo,BeginList,EndList);
            }
        }
        for(int CurrentStruct = 0; CurrentStruct < Grammar.Structs.size();CurrentStruct++)
        {
            std::string StructName = Grammar.Structs[CurrentStruct].Name;
            if(DepInfo.ChildrenMap[CurrentStruct].size() != 0)
            {
                StructName += "_Base";   
            }
            HeaderOut<<"template<> inline int MBCC::GetTypeBegin<" << StructName <<">(){return("<<std::to_string(BeginList[CurrentStruct])<<");}\n";
            HeaderOut<<"template<> inline int MBCC::GetTypeEnd<" << StructName <<">(){return("<<std::to_string(EndList[CurrentStruct])<<");}\n";
        }
    }
    void CPPParserGenerator::WriteLLParser(MBCCDefinitions const& Grammar,std::string const& HeaderName,MBUtility::MBOctetOutputStream& HeaderOut,MBUtility::MBOctetOutputStream& SourceOut,int k)
    {
        std::vector<bool> ERules = CalculateENonTerminals(Grammar); 
        ParserCompilerHandler::VerifyNotLeftRecursive(Grammar,ERules);
        GLA GrammarGLA(Grammar,k);
        std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> TotalProductions = ParserCompilerHandler::CalculateProductionsLinearApproxLOOK(Grammar,ERules,GrammarGLA,k);
        NonTerminalIndex NonTermIndex = 0;
        for(auto const& Productions : TotalProductions)
        {
            if(!RulesAreDisjunct(Productions))
            {
                throw std::runtime_error("Error creating linear-approximate-LL("+std::to_string(k)+") parser for grammar: Rule \""+Grammar.NonTerminals[NonTermIndex].Name+"\" is non deterministic");
            }
            NonTermIndex++;
        }
        CPPStreamIndenter HeaderIndent(&HeaderOut);
        CPPStreamIndenter SourceIndent(&SourceOut);
        p_WriteParser(Grammar,TotalProductions,HeaderName,HeaderIndent,SourceIndent);
    } 
    void CPPParserGenerator::WriteParser(MBCCDefinitions const& Grammar,std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> const& TotalProductions,std::string const& OutputBase)
    {
        std::ofstream HeaderFile = std::ofstream(OutputBase+".h",std::ios::out | std::ios::binary);
        if(!HeaderFile.is_open())
        {
            throw std::runtime_error("Error opening output header file \""+OutputBase+".h\"");
        }
        std::ofstream SourceFile = std::ofstream(OutputBase+".cpp",std::ios::out | std::ios::binary);
        if(!SourceFile.is_open())
        {
            throw std::runtime_error("Error opening output source file \""+OutputBase+".cpp\"");
        }
        std::string HeaderName = MBUnicode::PathToUTF8(std::filesystem::path(OutputBase).filename())+".h";
        MBUtility::MBFileOutputStream HeaderStream(&HeaderFile);
        MBUtility::MBFileOutputStream SourceStream(&SourceFile);
        CPPStreamIndenter HeaderIndent(&HeaderStream);
        CPPStreamIndenter SourceIndent(&SourceStream);
        p_WriteParser(Grammar,TotalProductions,HeaderName,HeaderIndent,SourceIndent);
    }
    void CPPParserGenerator::p_WriteParser(MBCCDefinitions const& Grammar,std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> const& ProductionsLOOk,
            std::string const& HeaderName,
        MBUtility::MBOctetOutputStream& HeaderOut,MBUtility::MBOctetOutputStream& SourceOut)
    {
        p_WriteSource(Grammar,ProductionsLOOk,HeaderName,SourceOut); 
        WriteHeader(Grammar,HeaderOut);
    }
    void CPPParserGenerator::WriteHeader(MBCCDefinitions const& Grammar, MBUtility::MBOctetOutputStream& HeaderOut)
    {

        HeaderOut << "#pragma once\n";
        HeaderOut << "#include <MBCC/MBCC.h>\n"; 
        HeaderOut << "#include <string>\n";
        HeaderOut << "#include <vector>\n";
        HeaderOut << "#include <stdexcept>\n";
        HeaderOut << "#include <variant>\n";
        HeaderOut << "#include <memory>\n";
        DependancyInfo DepInfo = Grammar.DepInfo;
        h_WriteStructures(Grammar,DepInfo,HeaderOut);
        p_WriteFunctionHeaders(Grammar,HeaderOut);
        HeaderOut<<"inline MBCC::Tokenizer GetTokenizer()\n{\nMBCC::Tokenizer ReturnValue("+CLikeParser::LiteralEscapeString(Grammar.SkipRegex)+",{"; 
        for(auto const& Terminal : Grammar.Terminals)
        {
            HeaderOut<<CLikeParser::LiteralEscapeString(Terminal.RegexDefinition)<<",";   
        }
        HeaderOut <<"});\nreturn(ReturnValue);\n}";
    }
    void CPPParserGenerator::p_WriteFunctionHeaders(MBCCDefinitions const& Grammar,MBUtility::MBOctetOutputStream& HeaderOut)
    {
        std::string ReturnValueType = "void";
        for(NonTerminalIndex NonTermIndex = 0; NonTermIndex < Grammar.NonTerminals.size();NonTermIndex++)
        {
            StructDefinition const* AssoicatedStruct = nullptr;
            NonTerminal const& AssociatedNonTerminal = Grammar.NonTerminals[NonTermIndex];
            if(AssociatedNonTerminal.AssociatedStruct != -1)
            {
                AssoicatedStruct = &Grammar.Structs[AssociatedNonTerminal.AssociatedStruct];    
            }
            if(AssoicatedStruct != nullptr)
            {
                ReturnValueType = AssoicatedStruct->Name;
            }
            MBUtility::WriteData(HeaderOut,ReturnValueType+" Parse"+AssociatedNonTerminal.Name+"(MBCC::Tokenizer& Tokenizer);\n");
            for(int i = 0; i < AssociatedNonTerminal.Rules.size();i++)
            {
                HeaderOut<<ReturnValueType<<" Parse"<<AssociatedNonTerminal.Name<<"_"<<std::to_string(i)<<"(MBCC::Tokenizer& Tokenizer);\n"; 
            }
        }
    }
    void CPPParserGenerator::p_WriteSource(MBCCDefinitions const& Grammar,std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> const& ProductionsLOOk,
        std::string const& HeaderName,MBUtility::MBOctetOutputStream& SourceOut)
    {
        SourceOut<<"#include \""<<HeaderName<<"\"\n";
        CLikeParser ParserGenerator;
        ParserGenerator.SetAdapter(this);
        ParserGenerator.WriteNonTerminalFunctions(Grammar,ProductionsLOOk,SourceOut);
    }

    std::string CPPParserGenerator::GetString(MBCCDefinitions const& Grammar,TypeInfo Type)
    {
        std::string ReturnValue;
        bool IsList = (Type & TypeFlags::List) != 0;
        Type = Type & (~TypeFlags::List);
        if(IsList)
        {
            ReturnValue = "std::vector<";
        }
        if(Builtin(Type))
        {
            ReturnValue += h_BuiltinToCPP(Type);
        }
        else
        {
            ReturnValue += Grammar.Structs[Type].Name; 
        }
        if(IsList) ReturnValue += ">"; 
        return(ReturnValue);
    }
    std::string CPPParserGenerator::GetTypeString(MBCCDefinitions const& Grammar,TypeInfo Type) 
    {
        return GetString(Grammar,Type);
    }
    std::string CPPParserGenerator::GetRHSString(MBCCDefinitions const& Grammar,RuleComponent const& ComponentToInspect) 
    {
        std::string ReturnValue;
        if(ComponentToInspect.IsTerminal)
        {
            if(ComponentToInspect.AssignedMember.IsEmpty())
            {
                return "";   
            }
            if(ComponentToInspect.AssignedMember.ResultType & TypeFlags::String)
            {
                return "Tokenizer.Peek().Value";
            }
            else if(ComponentToInspect.AssignedMember.ResultType & TypeFlags::Bool)
            {
                return "Tokenizer.Peek().Value == \"true\"";
            }
            else if(ComponentToInspect.AssignedMember.ResultType & TypeFlags::Int)
            {
                return "std::stoi(Tokenizer.Peek().Value)";
            }
        }
        if(ComponentToInspect.ReferencedRule.IsType<MemberReference>())
        {
            MemberReference const& Expr = ComponentToInspect.ReferencedRule.GetType<MemberReference>();
            if(Expr.Names[0] == "TOKEN")
            {
                return "Tokenizer.Peek()."+Expr.Names[1];
            }
            //no a terminal, and not a special rule, therefore a reference to a non-termial parse function + optional 
            //member references
            ReturnValue = "Parse"+Grammar.NonTerminals[ComponentToInspect.ComponentIndex].Name+"(Tokenizer)";
            for(int i = 1; i < Expr.Names.size();i++)
            {
                ReturnValue += "."+Expr.Names[i];
                if(!Builtin(Expr.PartTypes[i]) && Grammar.DepInfo.HasChildren(Expr.PartTypes[i]))
                {
                    ReturnValue += ".GetBase()";
                }
            }
        }
        else if(ComponentToInspect.ReferencedRule.IsType<Literal>())
        {
            if(ComponentToInspect.ReferencedRule.ResultType & TypeFlags::String)
            {
                ReturnValue = "\""+ComponentToInspect.ReferencedRule.GetType<Literal>().LiteralString+"\"";
            }
            else
            {
                ReturnValue = ComponentToInspect.ReferencedRule.GetType<Literal>().LiteralString;   
            }
        }
        else
        {
            assert(false && "p_GetRHSString doesn't cover all cases for ReferencedRule.GetType");   
        }
        return ReturnValue;
    }
    std::string CPPParserGenerator::GetLHSMember(MBCCDefinitions const& Grammar,NonTerminal const& AssociatedNonTerminal
    ,ParseRule const& Production,RuleComponent const& ComponentToInspect) 
    {
        std::string MemberAccessString;
        MemberReference const& Expr = ComponentToInspect.AssignedMember.GetType<MemberReference>();
        if(AssociatedNonTerminal.AssociatedStruct != -1 && Grammar.DepInfo.HasChildren(AssociatedNonTerminal.AssociatedStruct))
        {
            MemberAccessString += ".GetBase()";
        }
        for(int i = 0; i < Expr.Names.size();i++)
        {
            MemberAccessString  += "."+Expr.Names[i];
            if(i + 1 < Expr.Names.size() && !Builtin(Expr.PartTypes[i]) && Grammar.DepInfo.HasChildren(Expr.PartTypes[i]))
            {
                MemberAccessString += ".GetBase()";
            }
        }
        return MemberAccessString;
    }
    std::string CPPParserGenerator::GetThrowExpectedException(std::string const& CurrentRule,std::string const& ExpectedRule) 
    {
        return "throw MBCC::ParsingException(Tokenizer.Peek().Position,\""+CurrentRule+"\",\"" +ExpectedRule+"\");\n";
    }
    std::string CPPParserGenerator::GetFunctionArguments() 
    {
        return "MBCC::Tokenizer& Tokenizer";
    }
    std::string CPPParserGenerator::ListAddFunc() 
    {
        return "push_back";
    }
    std::string CPPParserGenerator::GetLookTableVariable(int TotalProductionSize,int LookDepth,int TerminalCount) 
    {
        return "const bool LOOKTable["+std::to_string(TotalProductionSize)+"]["+std::to_string(LookDepth)+"]["+std::to_string(TerminalCount)+"]";
    }
    std::string CPPParserGenerator::GetLookIndex(std::string First,std::string Second,std::string Third)
    {
        return "["+First+"]["+Second+"]["+Third+"]";
    }
}
