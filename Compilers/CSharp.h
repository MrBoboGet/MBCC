#include "../MBCC.h"

namespace MBCC
{
    class CSharpParserGenerator : public ParserCompiler
    {
        std::vector<std::vector<std::string>> m_ProductionPredicates;


        void p_WriteClasses(MBCCDefinitions const& Grammar,MBUtility::MBOctetOutputStream& OutStream);
    public:
        virtual void WriteParser(MBCCDefinitions const& Grammar,
                std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> const& TotalProductions,
                std::string const& OutputBase) override;
    };
}
