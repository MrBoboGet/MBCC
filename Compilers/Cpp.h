#include "../MBCC.h"

namespace MBCC
{
    class CPPParserGenerator
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
        void p_WriteNonTerminalProduction(MBCCDefinitions const& Grammar,NonTerminalIndex NonTerminal,int ProductionIndex,std::string const& FunctionName,MBUtility::MBOctetOutputStream& SourceOut);

        static std::string p_GetTypeString(MBCCDefinitions const& Grammar,TypeInfo Type);
    public:
        void WriteLLParser(MBCCDefinitions const& InfoToWrite,std::string const& HeaderName,MBUtility::MBOctetOutputStream& HeaderOut,MBUtility::MBOctetOutputStream& SourceOut,int k = 2);
    };
}
