#include "MBCC.h"
#include <MBCLI/MBCLI.h>
namespace MBCC
{
    class MBCC_CLI
    {
    private:

    public: 
        int Run(MBCLI::MBTerminal& AssociatedTerminal,int argc,const char* const* argv);
    };
};
