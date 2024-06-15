#include <MBUnicode/MBUnicode.h>
#include "CppIRParser.h"
#include "Cpp.h"


namespace MBCC
{
    std::string CppIRParser::p_ColumnToBoolArray(MBMath::MBDynamicMatrix<bool> const& Matrix, int k)
    {
        std::string ReturnValue = "{";       
        for(int i = 0; i < Matrix.NumberOfRows();i++)
        {
            if(Matrix(i,k))
            {
                ReturnValue += "true";
            }   
            else
            {
                ReturnValue += "false";   
            }
            ReturnValue += ",";
        } 
        ReturnValue += "}";
        return(ReturnValue);
    }
    void CppIRParser::p_WriteLOOKTable(MBUtility::MBOctetOutputStream& OutStream,LookType const& Look)
    {
        OutStream<<"const bool LOOKTable[";
        int TotalProductionSize = 0;
        int LOOKDepth = Look[0][0].NumberOfColumns();
        //SourceOut << "const bool LOOKTable[][][][] = {";
        std::string TableString = "";
        for(auto const& NonTerminalProductions : Look)
        {
            auto CombinedProductions = CombineProductions(NonTerminalProductions);
            TableString += "{";
            for(int k = 0; k < CombinedProductions.NumberOfColumns();k++)
            {
                TableString += p_ColumnToBoolArray(CombinedProductions,k);
                TableString += ",";
            }
            TableString += "},";
            TotalProductionSize += NonTerminalProductions.size()+1;
            for(auto const& Production : NonTerminalProductions)
            {
                TableString += "{";
                for(int k = 0; k < Production.NumberOfColumns();k++)
                {
                    TableString += p_ColumnToBoolArray(Production,k);
                    TableString += ",";
                }
                TableString += "},";
            } 
        }  
        //SourceOut << "const bool LOOKTable["+std::to_string(TotalProductionSize)+"]["+std::to_string(LOOKDepth)+"]["
        //    +std::to_string(ProductionsLOOk[0][0].NumberOfRows())+"] = {"<<TableString<<"};\n";
        OutStream << std::to_string(TotalProductionSize)<<"]["<<std::to_string(LOOKDepth)<< "]["<<
            std::to_string(Look[0][0].NumberOfRows()) << "] = {"<<TableString<<"};\n";
    }
    void CppIRParser::WriteParser(MBCCDefinitions const& Grammar,LookType const& TotalProductions,std::string const& OutputBase)
    {
        m_Grammar = &Grammar;
        std::ofstream HeaderFile = std::ofstream(OutputBase+".h",std::ios::out | std::ios::binary);
        if(!HeaderFile.is_open())
        {
            throw std::runtime_error("Error opening output header file \""+OutputBase+".h\"");
        }
        std::ofstream SourceFile = std::ofstream(OutputBase+".cpp",std::ios::out | std::ios::binary);
        if(!SourceFile.is_open())
        {
            throw std::runtime_error("Error opening output source file \""+OutputBase+".cpp\"");
        }
        std::string HeaderName = MBUnicode::PathToUTF8(std::filesystem::path(OutputBase).filename())+".h";
        MBUtility::MBFileOutputStream HeaderStream(&HeaderFile);
        MBUtility::MBFileOutputStream SourceStream(&SourceFile);
        StreamIndenter HeaderIndent(&HeaderStream,'{','}');
        m_SourceStream= std::make_unique<StreamIndenter>(&SourceStream,'{','}');

        *m_SourceStream<<"#include \""<<HeaderName<<"\"\n";
        CPPParserGenerator::WriteHeader(Grammar,HeaderStream);
        p_WriteLOOKTable(*m_SourceStream,TotalProductions);
        IROptions Options;
        std::vector<Function> ParserIR = ConvertToIR(Grammar,Options,TotalProductions);
        for(auto const& Function : ParserIR)
        {
            Traverse(*this,Function);
        }
    }
}
