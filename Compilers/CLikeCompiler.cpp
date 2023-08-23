#include "CLikeCompiler.h"


namespace MBCC
{
    std::string CLikeParser::p_GetUniqueName()
    {
        std::string ReturnValue = "MBCC_TempVar"+std::to_string(m_CurrentNameIndex);
        m_CurrentNameIndex++;
        return ReturnValue;
    }
    std::string CLikeParser::p_GetCondType(MBCCDefinitions const& Grammar,RuleComponent const& ComponentToInspect)
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
    std::string CLikeParser::p_GetCondExpression(MBCCDefinitions const& Grammar,RuleComponent const& ComponentToInspect)
    {
        std::string ReturnValue;
        if(ComponentToInspect.IsTerminal && !(ComponentToInspect.Max == 1 && ComponentToInspect.Min == 1))
        {
            ReturnValue = "Tokenizer.Peek().Type == " + std::to_string(ComponentToInspect.ComponentIndex);
        }
        else if(!ComponentToInspect.IsTerminal)
        {
            ReturnValue = p_GetLOOKPredicate(ComponentToInspect.ComponentIndex);
        }

        return ReturnValue;
    }

    std::string const& CLikeParser::p_GetLOOKPredicate(NonTerminalIndex AssociatedNonTerminal,int Production)
    {
        assert(AssociatedNonTerminal < m_ProductionPredicates.size() && (Production+1 < m_ProductionPredicates[AssociatedNonTerminal].size()));
        if(Production == -1)
        {
            return(m_ProductionPredicates[AssociatedNonTerminal][0]);
        } 
        return(m_ProductionPredicates[AssociatedNonTerminal][Production+1]);
    }
    std::string CLikeParser::p_ColumnToBoolArray(MBMath::MBDynamicMatrix<bool> const& Matrix, int k)
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
    void CLikeParser::p_WriteLOOKTable(std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> const& ProductionsLOOk,MBUtility::MBOctetOutputStream& SourceOut)
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
                TableString += p_ColumnToBoolArray(CombinedProductions,k);
                TableString += ",";
            }
            TableString += "},";
            TotalProductionSize += NonTerminalProductions.size()+1;
            for(auto const& Production : NonTerminalProductions)
            {
                TableString += "{";
                for(int k = 0; k < Production.NumberOfColumns();k++)
                {
                    TableString += p_ColumnToBoolArray(Production,k);
                    TableString += ",";
                }
                TableString += "},";
            } 
        }  
        //SourceOut << "const bool LOOKTable["+std::to_string(TotalProductionSize)+"]["+std::to_string(LOOKDepth)+"]["
        //    +std::to_string(ProductionsLOOk[0][0].NumberOfRows())+"] = {"<<TableString<<"};\n";
        SourceOut << m_Adapter->GetLookTableVariable(TotalProductionSize,LOOKDepth,ProductionsLOOk[0][0].NumberOfRows()) << " = {"<<TableString<<"};\n";
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
    std::string CLikeParser::p_GetLHSMember(MBCCDefinitions const& Grammar,NonTerminal const& AssociatedNonTerminal,ParseRule const& Production ,RuleComponent const& ComponentToInspect,DelayedInfo& Delayed)
    {
        std::string ReturnValue = m_Adapter->GetLHSMember(Grammar,AssociatedNonTerminal,Production,ComponentToInspect);
        if(!Production.NeedsAssignmentOrder)
        {
            ReturnValue = "ReturnValue"+ReturnValue;
        }
        else
        {
            std::string NewVarName = p_GetUniqueName();
            Delayed.AccessString = ReturnValue;
            ReturnValue = NewVarName;
            Delayed.TempVar  = NewVarName;
        }
        return ReturnValue;
    }
    void CLikeParser::p_WriteRuleComponent(MBCCDefinitions const& Grammar,
            NonTerminal const& AssociatedNonTerminal,
            ParseRule const& Production ,
            RuleComponent const& ComponentToWrite, 
            MBUtility::MBOctetOutputStream& OutStream, 
            std::vector<std::pair<std::string,std::string>>& DelayedAssignments)
    {
        DelayedInfo DelayedMember;
        std::string Body = p_GetBody(Grammar,AssociatedNonTerminal,Production,ComponentToWrite,DelayedMember);
        std::string CondType = p_GetCondType(Grammar,ComponentToWrite);
        std::string CondExpression = p_GetCondExpression(Grammar,ComponentToWrite);

        if(DelayedMember.TempVar != "")
        {
            DelayedAssignments.push_back( {DelayedMember.AccessString,DelayedMember.TempVar});
            OutStream<<m_Adapter->GetTypeString(Grammar,ComponentToWrite.AssignedMember.ResultType)<<" "<<DelayedMember.TempVar<<";\n";
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
    std::string CLikeParser::p_GetBody(MBCCDefinitions const& Grammar,NonTerminal const& AssociatedNonTerminal,ParseRule const& Production ,RuleComponent const& ComponentToWrite,DelayedInfo& Delayed)
    {
        std::string ReturnValue;
        //only check if not in an optional or * 
        if(ComponentToWrite.Max == 1 && ComponentToWrite.Min == 1 || (ComponentToWrite.Min == 1 && ComponentToWrite.Max == -1))
        {
            if(ComponentToWrite.IsTerminal)
            {
                ReturnValue = "if(Tokenizer.Peek().Type != " + std::to_string(ComponentToWrite.ComponentIndex) + ")\n{\n";
                ReturnValue += m_Adapter->GetThrowExpectedException(AssociatedNonTerminal.Name,Grammar.Terminals[ComponentToWrite.ComponentIndex].Name);
                ReturnValue += "\n}\n";
            }
            else
            {
                ReturnValue = "if(!(" + p_GetLOOKPredicate(ComponentToWrite.ComponentIndex) + "))\n{\n"+
                    m_Adapter->GetThrowExpectedException(AssociatedNonTerminal.Name,Grammar.Terminals[ComponentToWrite.ComponentIndex].Name)+
                    "\n}\n";
            }
        }
        std::string LhsString = p_GetLHSMember(Grammar,AssociatedNonTerminal,Production,ComponentToWrite,Delayed);
        std::string RhsString = m_Adapter->GetRHSString(Grammar,ComponentToWrite);
        if(!ComponentToWrite.AssignedMember.IsEmpty())
        {
            if( ComponentToWrite.Max == -1 || 
                    ((ComponentToWrite.AssignedMember.ResultType & TypeFlags::List) && !(ComponentToWrite.ReferencedRule.ResultType & TypeFlags::List)))
            {
                ReturnValue += LhsString+"." + m_Adapter->ListAddFunc() + "("+RhsString+");\n";
            }
            else
            {
                ReturnValue += LhsString + " = " + RhsString + ";\n";
            }
        }
        else
        { 
            ReturnValue += RhsString+";\n";
        }
        if(ComponentToWrite.IsTerminal)
        {
            ReturnValue += "Tokenizer.ConsumeToken();\n";
        }
        return ReturnValue;
    }

    void CLikeParser::p_WriteNonTerminalFunction(MBCCDefinitions const& Grammar,NonTerminalIndex NonTermIndex, MBUtility::MBOctetOutputStream& SourceOut)
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
        MBUtility::WriteData(SourceOut,ReturnValueType+" Parse"+AssociatedNonTerminal.Name+"(" + m_Adapter->GetFunctionArguments() +"\n{\n");
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
            SourceOut<<"else\n{\n"<<
                m_Adapter->GetThrowExpectedException(AssociatedNonTerminal.Name,AssociatedNonTerminal.Name)<<
                "\n}\n";

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
    void CLikeParser::p_WriteNonTerminalProduction(MBCCDefinitions const& Grammar,NonTerminalIndex NonTermIndex,int ProductionIndex,std::string const& FunctionName,MBUtility::MBOctetOutputStream& SourceOut)
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
        MBUtility::WriteData(SourceOut,ReturnValueType+" "+FunctionName+"(" + m_Adapter->GetFunctionArguments() + "\n{\n");
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
    void CLikeParser::WriteNonTerminalFunctions(MBCCDefinitions  const& Grammar,MBUtility::MBOctetOutputStream& SourceOut)
    {
       
    }
}
