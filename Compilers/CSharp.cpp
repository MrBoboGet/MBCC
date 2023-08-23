#include "CSharp.h"


namespace MBCC
{
    void CSharpParserGenerator::p_WriteClasses(MBCCDefinitions const& Grammar,MBUtility::MBOctetOutputStream& OutStream)
    {
        for(auto const& Struct : Grammar.Structs)
        {
            OutStream<<"class "<<Struct.Name;
            if(Struct.ParentStruct != "")
            {
                OutStream<<" : "<<Struct.ParentStruct;
            }
            OutStream<<"\n{\n";
            for(auto const& Member : Struct.MemberVariables)
            {
                if(Member.IsType<StructMemberVariable_List>())
                {
                    OutStream<<"List<"<<Member.GetType<StructMemberVariable_List>().ListType<<"> ";
                }
                else if(Member.IsType<StructMemberVariable_Bool>())
                {
                    OutStream<<"bool";
                }
                else if(Member.IsType<StructMemberVariable_String>())
                {
                    OutStream<<"string";
                }
                else if(Member.IsType<StructMemberVariable_Int>())
                {
                    OutStream<<"int";
                }
                else if(Member.IsType<StructMemberVariable_Struct>())
                {
                    OutStream<<Member.GetType<StructMemberVariable_Struct>().Name;
                }
                else if(Member.IsType<StructMemberVariable_Raw>())
                {
                    OutStream<<Member.GetType<StructMemberVariable_Raw>().Name;
                }
                else if(Member.IsType<StructMemberVariable_tokenPosition>())
                {
                    OutStream<<"TokenPosition";
                }
                OutStream << " "<<Member.GetName();
                if(Member.GetDefaultValue().Value != "")
                {
                    OutStream<<" = "<<Member.GetDefaultValue().Value;
                }
                OutStream<<";\n";
            }
            OutStream<<"\n}\n";
        }
    }
    void CSharpParserGenerator::WriteParser(MBCCDefinitions const& Grammar,
            std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> const& TotalProductions,
            std::string const& OutputBase) 
    {
        std::ofstream SourceFile = std::ofstream(OutputBase+".cs",std::ios::out | std::ios::binary);
        if(!SourceFile.is_open())
        {
            throw std::runtime_error("Error opening output source file \""+OutputBase+".cpp\"");
        }
        MBUtility::MBFileOutputStream SourceStream(&SourceFile);
        CPPStreamIndenter SourceIndent(&SourceStream);
        p_WriteClasses(Grammar,SourceIndent);
    }
}
