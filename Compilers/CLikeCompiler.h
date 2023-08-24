#pragma once
#include "../MBCC.h"

namespace MBCC
{
    struct DelayedInfo
    {
        std::string TempVar;
        std::string AccessString;
    };
    class CLikeAdapter
    {
    public:
        virtual std::string GetTypeString(MBCCDefinitions const& Grammar,TypeInfo Type) = 0;
        virtual std::string GetRHSString(MBCCDefinitions const& Grammar,RuleComponent const& ComponentToInspect) = 0;
        virtual std::string GetLHSMember(MBCCDefinitions const& Grammar,NonTerminal const& AssociatedNonTerminal
                ,ParseRule const& Production,RuleComponent const& ComponentToInspect) = 0;
        virtual std::string GetThrowExpectedException(std::string const& CurrentRule,std::string const& ExpectedRule) = 0;
        virtual std::string GetFunctionArguments() = 0;
        virtual std::string ListAddFunc() = 0;
        virtual std::string GetLookTableVariable(int TotalProductionSize,int LookDepth,int TerminalCount) = 0;
        virtual std::string GetLookIndex(std::string First,std::string Second,std::string Third) = 0;
        virtual std::string GetFunctionPrefix()
        {
            return "";   
        }
        virtual std::string VariableInitializationString(std::string const& VariableTypeString)
        {
            return "";   
        }

    };
    class CLikeParser
    {
        int m_CurrentNameIndex = 0;
        typedef CLikeAdapter T;
        T* m_Adapter = nullptr;
        std::vector<std::vector<std::string>> m_ProductionPredicates;


private:
        std::string const& p_GetLOOKPredicate(NonTerminalIndex AssociatedNonTerminal,int Production = -1);
        void p_WriteLOOKTable(std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> const& ProductionsLOOk,MBUtility::MBOctetOutputStream& SourceOut);
        std::string p_ColumnToBoolArray(MBMath::MBDynamicMatrix<bool> const& Matrix, int k);

        std::string p_GetLHSMember(MBCCDefinitions const& Grammar,NonTerminal const& AssociatedNonTerminal,ParseRule const& Production ,RuleComponent const& ComponentToInspect,DelayedInfo& Delayed);

        std::string p_GetUniqueName();
        std::string p_GetCondType(MBCCDefinitions const& Grammar,RuleComponent const& ComponentToInspect);
        std::string p_GetCondExpression(MBCCDefinitions const& Grammar,RuleComponent const& ComponentToInspect);

        std::string p_GetBody(MBCCDefinitions const& Grammar,NonTerminal const& AssociatedNonTerminal,ParseRule const& Production ,RuleComponent const& ComponentToInspect,DelayedInfo& Delayed);


        void p_WriteRuleComponent(MBCCDefinitions const& Grammar,
                                  NonTerminal const& AssociatedNonTerminal,
                                  ParseRule const& Production ,
                                  RuleComponent const& ComponentToWrite, 
                                  MBUtility::MBOctetOutputStream& OutStream, 
                                  std::vector<std::pair<std::string,std::string>>& DelayedAssignments);



        void p_WriteNonTerminalFunction(MBCCDefinitions const& Grammar,NonTerminalIndex NonTerminal, MBUtility::MBOctetOutputStream& SourceOut);
        void p_WriteNonTerminalProduction(MBCCDefinitions const& Grammar,NonTerminalIndex NonTerminal,int ProductionIndex,std::string const& FunctionName,MBUtility::MBOctetOutputStream& SourceOut);

public:
        void SetAdapter(CLikeAdapter* Adapter)
        {
            m_Adapter = Adapter;   
        }
        void WriteNonTerminalFunctions(MBCCDefinitions  const& Grammar,std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> const& ProductionsLOOk,MBUtility::MBOctetOutputStream& SourceOut);

        static std::string LiteralEscapeString(std::string const& StringToEscape)
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
            return("\"" +ReturnValue+"\"");
        }
    };
}
