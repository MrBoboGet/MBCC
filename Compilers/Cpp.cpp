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
    std::string h_CppLiteralEscapeString(std::string const& StringToEscape)
    {
        std::string ReturnValue;
        if(StringToEscape.empty())
        {
            return(ReturnValue);   
        }
        size_t ParseOffset = 0;
        while(ParseOffset < StringToEscape.size())
        {
            size_t NextSlash = StringToEscape.find('\\', ParseOffset);
            size_t NextQuote = StringToEscape.find('"',ParseOffset);
            size_t NextEscape = std::min(NextSlash,NextQuote);
            if(NextEscape == StringToEscape.npos)
            {
                ReturnValue.insert(ReturnValue.end(),StringToEscape.data()+ParseOffset,StringToEscape.data()+StringToEscape.size());
                break;
            }
            else
            {
                ReturnValue.insert(ReturnValue.end(),StringToEscape.data()+ParseOffset,StringToEscape.data()+NextEscape);
                ReturnValue += '\\';
                ReturnValue += StringToEscape[NextEscape];
                ParseOffset = NextEscape+1;
            }
        }
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
                if(Member.GetDefaultValue() != "")
                {
                    HeaderOut<<" = "<<Member.GetDefaultValue();   
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
        p_WriteHeader(Grammar,HeaderOut);
    }
    void CPPParserGenerator::p_WriteHeader(MBCCDefinitions const& Grammar, MBUtility::MBOctetOutputStream& HeaderOut)
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
        HeaderOut<<"inline MBCC::Tokenizer GetTokenizer()\n{\nMBCC::Tokenizer ReturnValue(\""+h_CppLiteralEscapeString(Grammar.SkipRegex)+"\",{"; 
        for(auto const& Terminal : Grammar.Terminals)
        {
            HeaderOut<<"\""<<h_CppLiteralEscapeString(Terminal.RegexDefinition)<<"\",";   
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
        p_WriteLOOKTable(ProductionsLOOk, SourceOut);
        //MBUtility::WriteData(SourceOut,"#include <MBParsing/MBCC.h>\n");
        //Order we write C/C++/C# implementations dont matter, we can just write them directly
        for(NonTerminalIndex i = 0; i < Grammar.NonTerminals.size();i++)
        {
            p_WriteNonTerminalFunction(Grammar,i,SourceOut);     
            for(int j = 0; j < Grammar.NonTerminals[i].Rules.size();j++)
            {
                p_WriteNonTerminalProduction(Grammar,i,j,"Parse"+Grammar.NonTerminals[i].Name+"_"+std::to_string(j),SourceOut); 
            }
        }       
    }
    //Probably the only function that needs to know about the special property that the look for all terminals are one greater than the 
    //number of non terminals, the last representing the EOF
    void CPPParserGenerator::p_WriteLOOKTable(std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> const& ProductionsLOOk,MBUtility::MBOctetOutputStream& SourceOut)
    {

        //Assumes that zero non terminals is invalid
        m_ProductionPredicates.reserve(ProductionsLOOk.size());
        //for each non termina + production, for each k for each terminal
        //the k+1 production is the combined productions, representing the whole non terminal
        int TotalProductionSize = 0;
        int LOOKDepth = ProductionsLOOk[0][0].NumberOfColumns();
        //SourceOut << "const bool LOOKTable[][][][] = {";
        std::string TableString = "";
        for(auto const& NonTerminalProductions : ProductionsLOOk)
        {
            auto CombinedProductions = CombineProductions(NonTerminalProductions);
            TableString += "{";
            for(int k = 0; k < CombinedProductions.NumberOfColumns();k++)
            {
                TableString += h_ColumnToBoolArray(CombinedProductions,k);
                TableString += ",";
            }
            TableString += "},";
            TotalProductionSize += NonTerminalProductions.size()+1;
            for(auto const& Production : NonTerminalProductions)
            {
                TableString += "{";
                for(int k = 0; k < Production.NumberOfColumns();k++)
                {
                    TableString += h_ColumnToBoolArray(Production,k);
                    TableString += ",";
                }
                TableString += "},";
            } 
        }  
        SourceOut << "const bool LOOKTable["+std::to_string(TotalProductionSize)+"]["+std::to_string(LOOKDepth)+"]["
            +std::to_string(ProductionsLOOk[0][0].NumberOfRows())+"] = {"<<TableString<<"};\n";
        int CurrentProductionIndex = 0;
        for(auto const& CurrentProduction : ProductionsLOOk)
        {
            std::vector<std::string> NewEntry;
            NewEntry.reserve(CurrentProduction.size()+1);
            std::string CombinedString = "LOOKTable["+std::to_string(CurrentProductionIndex)+"][0][Tokenizer.Peek().Type]";
            for(int k = 1; k < LOOKDepth;k++)
            {
                CombinedString +=  "&& LOOKTable["+std::to_string(CurrentProductionIndex)+"]["+std::to_string(k)+"][Tokenizer.Peek("+std::to_string(k)+").Type]";
            }
            CurrentProductionIndex += 1;
            assert(CombinedString.size() != 0);
            NewEntry.push_back(CombinedString);
            for(int j = 0; j < CurrentProduction.size();j++)
            {
                std::string NewString = "LOOKTable["+std::to_string(CurrentProductionIndex)+"][0][Tokenizer.Peek().Type]";
                for(int k = 1; k < LOOKDepth;k++)
                {
                    NewString +=  "&& LOOKTable["+std::to_string(CurrentProductionIndex)+"]["+std::to_string(k)+"][Tokenizer.Peek("+std::to_string(k)+").Type]";
                }
                assert(NewString.size() != 0);
                NewEntry.push_back(NewString);
                CurrentProductionIndex += 1;
            }
            assert(NewEntry.size() != 0);
            m_ProductionPredicates.push_back(std::move(NewEntry));
        }
    }
    std::string const& CPPParserGenerator::p_GetLOOKPredicate(NonTerminalIndex AssociatedNonTerminal,int Production)
    {
        assert(AssociatedNonTerminal < m_ProductionPredicates.size() && (Production+1 < m_ProductionPredicates[AssociatedNonTerminal].size()));
        if(Production == -1)
        {
            return(m_ProductionPredicates[AssociatedNonTerminal][0]);
        } 
        return(m_ProductionPredicates[AssociatedNonTerminal][Production+1]);
    }
    void CPPParserGenerator::p_WriteNonTerminalFunction(MBCCDefinitions const& Grammar,NonTerminalIndex NonTermIndex, MBUtility::MBOctetOutputStream& SourceOut)
    {
        std::string ReturnValueType = "void";
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
        MBUtility::WriteData(SourceOut,ReturnValueType+" Parse"+AssociatedNonTerminal.Name+"(MBCC::Tokenizer& Tokenizer)\n{\n");
        SourceOut<<ReturnValueType<<" ReturnValue;\n";
        if(AssociatedNonTerminal.Rules.size() > 1)
        {
            for(int i = 0; i < AssociatedNonTerminal.Rules.size();i++)
            {
                if(i != 0)
                {
                    MBUtility::WriteData(SourceOut,"else ");   
                }
                MBUtility::WriteData(SourceOut,"if "); 
                std::string const& LookPredicate = p_GetLOOKPredicate(NonTermIndex,i);
                assert(LookPredicate.size() != 0);
                MBUtility::WriteData(SourceOut,"("+LookPredicate+")\n{\n");    
                if(AssoicatedStruct != nullptr)
                {
                    MBUtility::WriteData(SourceOut,"ReturnValue = ");    
                }
                MBUtility::WriteData(SourceOut,"Parse"+AssociatedNonTerminal.Name+"_"+std::to_string(i)+"(Tokenizer);\n}\n");
            }
            SourceOut<<"else\n{\n throw MBCC::ParsingException(Tokenizer.Peek().Position,\""<<AssociatedNonTerminal.Name<<"\","<<
                "\""<<AssociatedNonTerminal.Name<<"\");\n}\n";

        }
        else
        {
            if(AssoicatedStruct != nullptr)
            {
                MBUtility::WriteData(SourceOut,"ReturnValue = ");    
            }
            MBUtility::WriteData(SourceOut,"Parse"+AssociatedNonTerminal.Name+"_"+std::to_string(0)+"(Tokenizer);\n");
        }
        if(AssoicatedStruct != nullptr)
        {
            MBUtility::WriteData(SourceOut,"return(ReturnValue);\n}\n");
        }
        else
        {
            MBUtility::WriteData(SourceOut,"}\n");
        }
    }
    std::string CPPParserGenerator::p_GetTypeString(MBCCDefinitions const& Grammar,TypeInfo Type)
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


    std::string CPPParserGenerator::p_GetUniqueName()
    {
        std::string ReturnValue = "MBCC_TempVar"+std::to_string(m_CurrentNameIndex);
        m_CurrentNameIndex++;
        return ReturnValue;
    }
    std::string CPPParserGenerator::p_GetRHSString(MBCCDefinitions const& Grammar,RuleComponent const& ComponentToInspect)
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
    std::string CPPParserGenerator::p_GetLHSMember(MBCCDefinitions const& Grammar,NonTerminal const& AssociatedNonTerminal,ParseRule const& Production ,RuleComponent const& ComponentToInspect,std::string& Delayed)
    {
        std::string ReturnValue;
        if(ComponentToInspect.AssignedMember.IsEmpty())
        {
            return "";   
        }
        std::string MemberAccessString;
        MemberReference const& Expr = ComponentToInspect.AssignedMember.GetType<MemberReference>();
        if(AssociatedNonTerminal.AssociatedStruct != -1 && Grammar.DepInfo.HasChildren(AssociatedNonTerminal.AssociatedStruct))
        {
            MemberAccessString += ".GetBase()";
        }
        for(int i = 0; i < Expr.Names.size();i++)
        {
            MemberAccessString  += "."+Expr.Names[i];
            if(!Builtin(Expr.PartTypes[i]) && Grammar.DepInfo.HasChildren(Expr.PartTypes[i]))
            {
                MemberAccessString += ".GetBase()";
            }
        }
        if(!Production.NeedsAssignmentOrder)
        {
            ReturnValue = "ReturnValue"+MemberAccessString;
        }
        else
        {
            std::string NewVarName = p_GetUniqueName();
            ReturnValue = p_GetTypeString(Grammar,ComponentToInspect.ReferencedRule.ResultType)+ " "+NewVarName+";\n";
            Delayed = MemberAccessString;
        }
        return ReturnValue;
    }
    std::string CPPParserGenerator::p_GetCondType(MBCCDefinitions const& Grammar,RuleComponent const& ComponentToInspect)
    {
        if(ComponentToInspect.Max == 1 && ComponentToInspect.Min == 0)
        {
            return "if";
        }
        else if(ComponentToInspect.Min == 0 && ComponentToInspect.Max == -1)
        {
            return "while";   
        }
        else if(ComponentToInspect.Min == 1 && ComponentToInspect.Max == -1)
        {
            return "do";   
        }
        return "";
    }
    std::string CPPParserGenerator::p_GetCondExpression(MBCCDefinitions const& Grammar,RuleComponent const& ComponentToInspect)
    {
        std::string ReturnValue;
        if(ComponentToInspect.IsTerminal && !(ComponentToInspect.Max == 1 && ComponentToInspect.Min == 1))
        {
            ReturnValue = "Tokenizer.Peek().Type == " + std::to_string(ComponentToInspect.ComponentIndex) + ")";
        }
        else
        {
            ReturnValue = p_GetLOOKPredicate(ComponentToInspect.ComponentIndex);
        }

        return ReturnValue;
    }
    std::string CPPParserGenerator::p_GetBody(MBCCDefinitions const& Grammar,NonTerminal const& AssociatedNonTerminal,ParseRule const& Production ,RuleComponent const& ComponentToWrite,std::string& DelayedString)
    {
        std::string ReturnValue;
        std::string LhsString = p_GetLHSMember(Grammar,AssociatedNonTerminal,Production,ComponentToWrite,DelayedString);
        std::string RhsString = p_GetRHSString(Grammar,ComponentToWrite);
        if(!ComponentToWrite.AssignedMember.IsEmpty())
        {
            if( ComponentToWrite.Max == -1 || 
                    ((ComponentToWrite.AssignedMember.ResultType & TypeFlags::List) && !(ComponentToWrite.ReferencedRule.ResultType & TypeFlags::List)))
            {
                ReturnValue = LhsString+".push_back("+RhsString+");\n";
            }
            else
            {
                ReturnValue = LhsString + " = " + RhsString + ";\n";
            }
        }
        else if(ComponentToWrite.IsTerminal && (ComponentToWrite.Max == 1 && ComponentToWrite.Min == 1))
        { 
            ReturnValue = "if(Tokenizer.Peek().Type != " + std::to_string(ComponentToWrite.ComponentIndex) +
                ")\n{\nthrow MBCC::ParsingException(Tokenizer.Peek().Position,\""+AssociatedNonTerminal.Name+"\",\""
                +Grammar.Terminals[ComponentToWrite.ComponentIndex].Name +"\""+
                    ");\n}\n";
        }
        if(ComponentToWrite.IsTerminal)
        {
            ReturnValue += "Tokenizer.ConsumeToken();\n";
        }
        return ReturnValue;
    }
    void CPPParserGenerator::p_WriteRuleComponent(MBCCDefinitions const& Grammar,NonTerminal const& AssociatedNonTerminal,ParseRule const& Production ,RuleComponent const& ComponentToWrite, 
            MBUtility::MBOctetOutputStream& OutStream, std::vector<std::pair<std::string,std::string>>& DelayedAssignments)
    {
        std::string DelayedMember;
        std::string Body = p_GetBody(Grammar,AssociatedNonTerminal,Production,ComponentToWrite,DelayedMember);
        std::string CondType = p_GetCondType(Grammar,ComponentToWrite);
        std::string CondExpression = p_GetCondExpression(Grammar,ComponentToWrite);

        if(DelayedMember != "")
        {
            DelayedAssignments.push_back( {DelayedMember,ComponentToWrite.AssignedMember.GetType<MemberReference>().Names[0]});
            OutStream<<p_GetTypeString(Grammar,ComponentToWrite.AssignedMember.ResultType)<<" "<<ComponentToWrite.AssignedMember.GetType<MemberReference>().Names[0]<<";\n";
        }
        if(CondType == "")
        {
            OutStream<<Body;   
        }
        else if(CondType == "if" || CondType == "while")
        {
            OutStream <<  CondType << "("<<CondExpression<<")\n{\n"<<Body<<"\n}\n";
        }
        else if(CondType == "do")
        {
            OutStream <<  CondType << "\n{\n"<<Body<<"\n}\n"<<"while("<<CondExpression<<")\n";
        }
        else
        {
            assert(false && "Unrecognzied cond type in p_WriteRuleComponent");   
        }
    }
    //TODO clean up this gigantic mess...
    void CPPParserGenerator::p_WriteNonTerminalProduction(MBCCDefinitions const& Grammar,NonTerminalIndex NonTermIndex,int ProductionIndex,std::string const& FunctionName,MBUtility::MBOctetOutputStream& SourceOut)
    {
        std::string ReturnValueType = "void";
        StructDefinition const* AssoicatedStruct = nullptr;
        NonTerminal const& AssociatedNonTerminal = Grammar.NonTerminals[NonTermIndex];
        ParseRule const& Production = AssociatedNonTerminal.Rules[ProductionIndex];
        if(AssociatedNonTerminal.AssociatedStruct != -1)
        {
            AssoicatedStruct = &Grammar.Structs[AssociatedNonTerminal.AssociatedStruct];    
        }
        if(AssoicatedStruct != nullptr)
        {
            ReturnValueType = AssoicatedStruct->Name;
        }
        MBUtility::WriteData(SourceOut,ReturnValueType+" "+FunctionName+"(MBCC::Tokenizer& Tokenizer)\n{\n");
        if(AssociatedNonTerminal.AssociatedStruct != -1)
        {
            MBUtility::WriteData(SourceOut,AssoicatedStruct->Name + " ReturnValue;\n");
        }
        //first = member, second = name
        std::vector<std::pair<std::string,std::string>> DelayedAssignments;
        int RegularComponentCount = 0;
        int MetaComponentCount = 0;
        for(int i = 0; i < Production.MetaComponents.size() + Production.Components.size();i++)
        {
            RuleComponent const* ComponentPointer = nullptr;
            if(MetaComponentCount < Production.MetaComponents.size() && i == Production.MetaComponents[MetaComponentCount].first)
            {
                ComponentPointer = &Production.MetaComponents[MetaComponentCount].second;
                MetaComponentCount++;
            }
            else
            {
                ComponentPointer = &Production.Components[RegularComponentCount];
                RegularComponentCount++;   
            }
            auto const& Component = *ComponentPointer;
            p_WriteRuleComponent(Grammar,AssociatedNonTerminal,Production,Component,SourceOut,DelayedAssignments);
        }
        for(auto const& Assignment : DelayedAssignments)
        {
            SourceOut<<"ReturnValue"<<Assignment.first<<" = std::move("<<Assignment.second<<");\n";
        }
        for(auto const& Action : Grammar.NonTerminals[NonTermIndex].Rules[ProductionIndex].Actions)
        {
            SourceOut<<Action.ActionString<<"\n";
        }
        MBUtility::WriteData(SourceOut,"return(ReturnValue);\n}\n");
    }
}
