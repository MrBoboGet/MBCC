#include "../MBCC.h"

namespace MBCC
{
    class CPPParserGenerator : public ParserCompiler
    {
        std::vector<std::vector<std::string>> m_ProductionPredicates;

        void p_WriteParser(MBCCDefinitions const& Grammar,std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> const& ProductionsLOOk,
                std::string const& HeaderName,
                MBUtility::MBOctetOutputStream& HeaderOut,MBUtility::MBOctetOutputStream& SourceOut);
        void p_WriteHeader(MBCCDefinitions const& Grammar, MBUtility::MBOctetOutputStream& HeaderOut);
        void p_WriteFunctionHeaders(MBCCDefinitions const& Grammar,MBUtility::MBOctetOutputStream& HeaderOut);
        void p_WriteSource(MBCCDefinitions const& Grammar,std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> const& ProductionsLOOk,
                std::string const& HeaderName,MBUtility::MBOctetOutputStream& SourceOut);
        void p_WriteLOOKTable(std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> const& ProductionsLOOk,MBUtility::MBOctetOutputStream& SourceOut);
        std::string const& p_GetLOOKPredicate(NonTerminalIndex AssociatedNonTerminal,int Production = -1);
        void p_WriteNonTerminalFunction(MBCCDefinitions const& Grammar,NonTerminalIndex NonTerminal, MBUtility::MBOctetOutputStream& SourceOut);

        int m_CurrentNameIndex = 0;

        struct DelayedInfo
        {
            std::string TempVar;
            std::string AccessString;
        };
        std::string p_GetUniqueName();
        std::string p_GetRHSString(MBCCDefinitions const& Grammar,RuleComponent const& ComponentToInspect);
        std::string p_GetLHSMember(MBCCDefinitions const& Grammar,NonTerminal const& AssociatedNonTerminal,ParseRule const& Production ,RuleComponent const& ComponentToInspect,DelayedInfo& Delayed);
        std::string p_GetCondType(MBCCDefinitions const& Grammar,RuleComponent const& ComponentToInspect);
        std::string p_GetCondExpression(MBCCDefinitions const& Grammar,RuleComponent const& ComponentToInspect);

        std::string p_GetBody(MBCCDefinitions const& Grammar,NonTerminal const& AssociatedNonTerminal,ParseRule const& Production ,RuleComponent const& ComponentToInspect,DelayedInfo& Delayed);


        void p_WriteRuleComponent(MBCCDefinitions const& Grammar,
                                  NonTerminal const& AssociatedNonTerminal,
                                  ParseRule const& Production ,
                                  RuleComponent const& ComponentToWrite, 
                                  MBUtility::MBOctetOutputStream& OutStream, 
                                  std::vector<std::pair<std::string,std::string>>& DelayedAssignments);

        void p_WriteNonTerminalProduction(MBCCDefinitions const& Grammar,NonTerminalIndex NonTerminal,int ProductionIndex,std::string const& FunctionName,MBUtility::MBOctetOutputStream& SourceOut);

        static std::string p_GetTypeString(MBCCDefinitions const& Grammar,TypeInfo Type);
    public:
        void WriteLLParser(MBCCDefinitions const& InfoToWrite,std::string const& HeaderName,MBUtility::MBOctetOutputStream& HeaderOut,MBUtility::MBOctetOutputStream& SourceOut,int k = 2);
        virtual void WriteParser(MBCCDefinitions const& Grammar,std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> const& TotalProductions,std::string const& OutputBase) override;
    };
}
