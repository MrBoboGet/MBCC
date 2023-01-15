#include "MBCC_CLI.h"

int main(int argc,char** argv)
{
    //char* NewArgv[] = { "mbcc","C:\\Users\\emanu\\Desktop\\Program\\C++\\MBObjectSpec\\ObjectSpec_BNF.mbnf","-s:Temp.cpp","-h:Temp.h" };
    //argv = NewArgv;
    //argc = sizeof(NewArgv) / sizeof(char*);
    MBCC::MBCC_CLI CLIHandler;
    MBCLI::MBTerminal Terminal;
    return(CLIHandler.Run(Terminal,argc,argv));
}
