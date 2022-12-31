#include "MBCC.h"
#include <MBCLI/MBCLI.h>
namespace MBCC
{
    struct ParserOptions
    {
        int k = 1;      
    };
    class MBCC_CLI
    {
    private:
        
        int p_Verify(MBCC::MBCCDefinitions const& Grammar,ParserOptions const& ParsOpts,MBCLI::ArgumentListCLIInput const& CLIInput,MBCLI::MBTerminal& Terminal);
        int p_Compile(MBCC::MBCCDefinitions const& Grammar,ParserOptions const& ParsOpts,MBCLI::ArgumentListCLIInput const& CLIInput,MBCLI::MBTerminal& Terminal);
    public: 
        int Run(MBCLI::MBTerminal& AssociatedTerminal,int argc,const char* const* argv);
    };
};
