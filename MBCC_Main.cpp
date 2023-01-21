#include "MBCC_CLI.h"

int main(int argc,char** argv)
{
    //char* NewArgv[] = { "mbcc","..\\..\\../MBObjectSpec/ObjectSpec_BNF.mbnf","-s:Temp.cpp","-h:Temp.h","-k:2"};
    //argv = NewArgv;
    //argc = sizeof(NewArgv) / sizeof(char*);
    MBCC::MBCC_CLI CLIHandler;
    MBCLI::MBTerminal Terminal;
    return(CLIHandler.Run(Terminal,argc,argv));
}
