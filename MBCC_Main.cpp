#include "MBCC_CLI.h"

int main(int argc,char** argv)
{
    MBCC::MBCC_CLI CLIHandler;
    MBCLI::MBTerminal Terminal;
    return(CLIHandler.Run(Terminal,argc,argv));
}
