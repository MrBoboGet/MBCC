#include "MBCC_CLI.h"
#include <MBUtility/MBFiles.h>
#include <fstream>
namespace MBCC
{
    void PrintProductions(std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> const& TotalProductions,MBCCDefinitions const& Grammar)
    {
        int NonTermIndex = 0;
        for(auto const& NonTerm : Grammar.NonTerminals)
        {
            std::cout<<"NonTerminal lookahead: "<<NonTerm.Name<<std::endl;
            auto Productions = TotalProductions[NonTermIndex];
            int ProductionIndex = 0;
            for(auto const& LookaheadInfo : Productions)
            {
                std::cout<<"Production "<< ProductionIndex<<":"<<std::endl;
                for(int i = 0; i < LookaheadInfo.NumberOfColumns();i++)
                {
                    std::cout<<"Level "<<std::to_string(i+1)<<":"<<std::endl;
                    //EOF is encoded as TerminalCount+1
                    for(int j = 0; j < Grammar.Terminals.size();j++)
                    {
                        if(LookaheadInfo(j,i))
                        {
                            std::cout<<Grammar.Terminals[j].Name<<"("<<j<<")"<<" ";   
                        }
                    }    
                    std::cout << std::endl;
                }
                ProductionIndex++;
                std::cout<<std::endl;
            }
            NonTermIndex++;
        }
    }
    int MBCC_CLI::p_Verify(MBCC::MBCCDefinitions const& Grammar,ParserOptions const& ParsOpts,MBCLI::ArgumentListCLIInput const& CLIInput,MBCLI::MBTerminal& AssociatedTerminal)
    {
        int ReturnValue = 0;
        //Always verify not left recursive
        auto ERules = MBCC::CalculateENonTerminals(Grammar);
        try
        {
            MBCC::LLParserGenerator::VerifyNotLeftRecursive(Grammar, ERules);
            if(CLIInput.CommandOptions.find("erules") != CLIInput.CommandOptions.end())
            {
                AssociatedTerminal.PrintLine("ERules: ");
                for(MBCC::NonTerminalIndex NonTermIndex = 0; NonTermIndex < Grammar.NonTerminals.size();NonTermIndex++)
                {
                    if(ERules[NonTermIndex])
                    {
                        AssociatedTerminal.PrintLine(Grammar.NonTerminals[NonTermIndex].Name);
                    }
                }
                AssociatedTerminal.PrintLine("");
            }
            MBCC::GLA GrammarGLA(Grammar,ParsOpts.k);
            auto Productions = MBCC::LLParserGenerator::CalculateProductionsLinearApproxLOOK(Grammar,ERules,GrammarGLA,ParsOpts.k);
            if(CLIInput.CommandOptions.find("productions") != CLIInput.CommandOptions.end())
            {
                PrintProductions(Productions,Grammar);
            }
        }
        catch(std::exception const& e)
        {
            AssociatedTerminal.PrintLine("Error found in grammar: "+std::string(e.what()));   
            ReturnValue = 1;
        }
        return(ReturnValue);
    }
    int MBCC_CLI::p_Compile(MBCC::MBCCDefinitions const& Grammar,ParserOptions const& ParsOpts,MBCLI::ArgumentListCLIInput const& Input,MBCLI::MBTerminal& AssociatedTerminal)
    {
        int ReturnValue = 0;
        std::string HeaderFile;
        std::string SourceFile;
        if(Input.CommandArgumentOptions.find("h") == Input.CommandArgumentOptions.end())
        {
            AssociatedTerminal.PrintLine("Parser generation requires the path for the output header file specified with the -h option");   
            return(1);
        }
        if(Input.CommandArgumentOptions.at("h").size() > 1)
        {
            AssociatedTerminal.PrintLine("Can only specify one output header file");    
            return(1);
        }
        HeaderFile = Input.CommandArgumentOptions.at("h")[0];
        if(Input.CommandArgumentOptions.find("s") == Input.CommandArgumentOptions.end())
        {
            AssociatedTerminal.PrintLine("Parser generation requires the path for the output source file specified with -s option");   
            return(1);
        }
        if(Input.CommandArgumentOptions.at("s").size() > 1)
        {
            AssociatedTerminal.PrintLine("Can only specify one output source file");    
            return(1);
        }
        SourceFile = Input.CommandArgumentOptions.at("s")[0];
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
        MBCC::LLParserGenerator LLGenerator;
        LLGenerator.WriteLLParser(Grammar,HeaderFile,HeaderStream,SourceStream,ParsOpts.k);
        return(ReturnValue);
    }
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
            std::filesystem::path FilePath = Input.CommandArguments[0];
            if(!std::filesystem::exists(FilePath))
            {
                AssociatedTerminal.PrintLine("bnf file \""+Input.CommandArguments[0]+"\" doesn't exist");    
                return(1);
            }
            std::string TotalFileData = MBUtility::ReadWholeFile(Input.CommandArguments[0]);
            MBCC::MBCCDefinitions Grammar;
            Grammar = MBCC::MBCCDefinitions::ParseDefinitions(TotalFileData.data(),TotalFileData.size(),0);
            ParserOptions ParsOpts;
            if(Input.CommandArgumentOptions.find("k") != Input.CommandArgumentOptions.end())
            {
                auto const& KVector = Input.CommandArgumentOptions.at("k");    
                if(KVector.size() > 1)
                {
                    AssociatedTerminal.PrintLine("Can only specify one k");    
                    return(1);
                }
                try
                {
                    ParsOpts.k = std::stoi(KVector[0]);    
                }
                catch(std::exception const& e)
                {
                    AssociatedTerminal.PrintLine("Error parsing -k option as an integer"); 
                    return(1);
                }
            }
            if(Input.CommandOptions.find("verify") != Input.CommandOptions.end())
            {
                p_Verify(Grammar,ParsOpts,Input,AssociatedTerminal);
            }
            else
            {
                p_Compile(Grammar,ParsOpts,Input,AssociatedTerminal);
            }
        }
        catch(std::exception const& e)
        {
            AssociatedTerminal.PrintLine(e.what());    
            return(1);
        }
        return(ReturnValue);
    }

};
