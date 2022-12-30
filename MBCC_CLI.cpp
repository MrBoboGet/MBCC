#include "MBCC_CLI.h"
#include <MBUtility/MBFiles.h>
#include <fstream>
namespace MBCC
{
    int MBCC_CLI::Run(MBCLI::MBTerminal& AssociatedTerminal,int argc,const char* const* argv)
    {
        int ReturnValue = 0;           
        MBCLI::ArgumentListCLIInput Input = MBCLI::ArgumentListCLIInput(argc,argv);        
        if(Input.CommandArguments.size() == 0)
        {
            AssociatedTerminal.PrintLine("mbcc requries the bnf filepath as the first argument");   
            return(1);
        }
        try
        {
            std::string HeaderFile;
            std::string SourceFile;
            if(Input.CommandArgumentOptions.find("h") == Input.CommandArgumentOptions.end())
            {
                AssociatedTerminal.PrintLine("Parser generation requires the path for the output header file specified with the -h option");   
                return(1);
            }
            if(Input.CommandArgumentOptions["h"].size() > 1)
            {
                AssociatedTerminal.PrintLine("Can only specify one output header file");    
                return(1);
            }
            HeaderFile = Input.CommandArgumentOptions["h"][0];
            if(Input.CommandArgumentOptions.find("s") == Input.CommandArgumentOptions.end())
            {
                AssociatedTerminal.PrintLine("Parser generation requires the path for the output source file specified with -s option");   
                return(1);
            }
            if(Input.CommandArgumentOptions["s"].size() > 1)
            {
                AssociatedTerminal.PrintLine("Can only specify one output source file");    
                return(1);
            }
            SourceFile = Input.CommandArgumentOptions["s"][0];
            std::filesystem::path FilePath = Input.CommandArguments[0];
            if(!std::filesystem::exists(FilePath))
            {
                AssociatedTerminal.PrintLine("bnf file \""+Input.CommandArguments[0]+"\" doesn't exist");    
                return(1);
            }
            std::ofstream OutHeader = std::ofstream(HeaderFile);
            std::ofstream OutSource = std::ofstream(SourceFile);
            if(!OutHeader.is_open())
            {
                AssociatedTerminal.PrintLine("Failed to open header file");    
                return(1);
            }
            if(!OutSource.is_open())
            {
                AssociatedTerminal.PrintLine("Failed to open output source file");    
                return(1);
            }
            MBUtility::MBFileOutputStream HeaderStream(&OutHeader);
            MBUtility::MBFileOutputStream SourceStream(&OutSource);
            std::string TotalFileData = MBUtility::ReadWholeFile(Input.CommandArguments[0]);
            MBCC::MBCCDefinitions Grammar;
            Grammar = MBCC::MBCCDefinitions::ParseDefinitions(TotalFileData.data(),TotalFileData.size(),0);
            MBCC::LLParserGenerator LLGenerator;
            int k = 1;
            if(Input.CommandArgumentOptions.find("k") != Input.CommandArgumentOptions.end())
            {
                auto const& KVector = Input.CommandArgumentOptions["k"];    
                if(KVector.size() > 1)
                {
                    AssociatedTerminal.PrintLine("Can only specify one k");    
                    return(1);
                }
                try
                {
                    k = std::stoi(KVector[0]);    
                }
                catch(std::exception const& e)
                {
                    AssociatedTerminal.PrintLine("Error parsing -k option as an integer"); 
                    return(1);
                }
            }
            LLGenerator.WriteLLParser(Grammar,HeaderFile,HeaderStream,SourceStream,k);
        }
        catch(std::exception const& e)
        {
            AssociatedTerminal.PrintLine(e.what());    
            return(1);
        }
        return(ReturnValue);
    }

};
