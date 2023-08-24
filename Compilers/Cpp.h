#include "../MBCC.h"
#include "CLikeCompiler.h"
namespace MBCC
{
    class CPPParserGenerator : public ParserCompiler, CLikeAdapter
    {
        std::vector<std::vector<std::string>> m_ProductionPredicates;

        void p_WriteParser(MBCCDefinitions const& Grammar,std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> const& ProductionsLOOk,
                std::string const& HeaderName,
                MBUtility::MBOctetOutputStream& HeaderOut,MBUtility::MBOctetOutputStream& SourceOut);
        void p_WriteHeader(MBCCDefinitions const& Grammar, MBUtility::MBOctetOutputStream& HeaderOut);
        void p_WriteFunctionHeaders(MBCCDefinitions const& Grammar,MBUtility::MBOctetOutputStream& HeaderOut);
        void p_WriteSource(MBCCDefinitions const& Grammar,std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> const& ProductionsLOOk,
                std::string const& HeaderName,MBUtility::MBOctetOutputStream& SourceOut);



        virtual std::string GetTypeString(MBCCDefinitions const& Grammar,TypeInfo Type) override;
        virtual std::string GetRHSString(MBCCDefinitions const& Grammar,RuleComponent const& ComponentToInspect) override;
        virtual std::string GetLHSMember(MBCCDefinitions const& Grammar,NonTerminal const& AssociatedNonTerminal
                ,ParseRule const& Production,RuleComponent const& ComponentToInspect) override;
        virtual std::string GetThrowExpectedException(std::string const& CurrentRule,std::string const& ExpectedRule) override;
        virtual std::string GetFunctionArguments() override;
        virtual std::string ListAddFunc() override;
        virtual std::string GetLookTableVariable(int TotalProductionSize,int LookDepth,int TerminalCount) override;
        virtual std::string GetLookIndex(std::string First,std::string Second,std::string Third) override;





    public:
        void WriteLLParser(MBCCDefinitions const& InfoToWrite,std::string const& HeaderName,MBUtility::MBOctetOutputStream& HeaderOut,MBUtility::MBOctetOutputStream& SourceOut,int k = 2);
        virtual void WriteParser(MBCCDefinitions const& Grammar,std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> const& TotalProductions,std::string const& OutputBase) override;
    };
}
