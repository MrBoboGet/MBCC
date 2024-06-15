#include "MBLisp.h"
namespace MBCC
{
    void LispParser::p_WriteLOOKTable(MBUtility::MBOctetOutputStream& OutStream,LOOKInfo const& Look)
    {
       OutStream<<"(set LOOKTable (list ";
       for(auto const& L1 : Look.Data)
       {
           OutStream<<"(list ";
           for(auto const& L2 : L1)
           {
               OutStream<<"(list";
               for(auto Value : L2)
               {
                   OutStream<< (Value ? " true" : " false");
               }
               OutStream<<") ";
           }
           OutStream<<") ";
       }
       OutStream<<"))";
    }
    void LispParser::p_WriteStructs(MBUtility::MBOctetOutputStream& OutStream,MBCCDefinitions const& Grammar)
    {
        for(auto const& Struct : Grammar.Structs)
        {
            OutStream<<"(defclass "<<Struct.Name<<"("<<Struct.ParentStruct<<")\t\n";
            for(auto const& Field : Struct.MemberVariables)
            {
                OutStream<<"("<<Field.GetBase().Name<<" ";
                if(Field.IsType<StructMemberVariable_List>())
                {
                    OutStream<<"(list)";
                }
                else if(Field.GetBase().DefaultValue.Value != "")
                {
                    OutStream<<Field.GetBase().DefaultValue.Value;
                }
                else if(Field.IsType<StructMemberVariable_Struct>())
                {
                    auto const& Struct= Field.GetType<StructMemberVariable_Struct>();
                    OutStream<<"("<<Struct.StructType<<")";
                }
                else
                {
                    OutStream<< "null";   
                }
                OutStream<<")\n";
            }
            OutStream<<"\n\v)\n";
        }
        OutStream<<"(defun get-tokenizer ()\t\n";
        OutStream<<"(tokenizer "<< CLikeParser::LiteralEscapeString(Grammar.SkipRegex);
        OutStream<<"(list";
        for(auto const& Token : Grammar.Terminals)
        {
            OutStream<<" "<<CLikeParser::LiteralEscapeString(Token.RegexDefinition);
        }
        OutStream<<")";
        OutStream<<")\n\v)\n";
    }
    void LispParser::WriteParser(MBCCDefinitions const& Grammar,LookType const& TotalProductions,std::string const& OutputBase)
    {
        std::ofstream OutStream(OutputBase+".lisp");
        MBUtility::MBFileOutputStream FileStream(&OutStream);
        WriteParser(Grammar,TotalProductions,FileStream);
    }
    void LispParser::WriteParser(MBCCDefinitions const& Grammar,LookType const& TotalProductions,MBUtility::MBOctetOutputStream& Stream)
    {
        StreamIndenter Indenter(&Stream,'\t','\v',true);
        m_Grammar = &Grammar;
        auto Info = GetLookInfo(TotalProductions);
        Indenter << "(import text)\n";

        m_SourceStream = &Indenter;
        p_WriteStructs(Indenter,Grammar);
        IROptions Options;
        Options.FillPolymorphic = false;
        std::vector<Function> ParserIR = ConvertToIR(Grammar,Options,TotalProductions);
        for(auto const& Function : ParserIR)
        {
            Traverse(*this,Function);
        }
        p_WriteLOOKTable(Indenter,Info);
    }
}
