#include "CSharp.h"


namespace MBCC
{
    void CSharpParserGenerator::p_WriteClasses(MBCCDefinitions const& Grammar,MBUtility::MBOctetOutputStream& OutStream)
    {
        for(auto const& Struct : Grammar.Structs)
        {
            OutStream<<"public class "<<Struct.Name;
            if(Struct.ParentStruct != "")
            {
                OutStream<<" : "<<Struct.ParentStruct;
            }
            OutStream<<"\n{\n";
            for(auto const& Member : Struct.MemberVariables)
            {
                OutStream<<"public ";
                std::string VariableType;
                if(Member.IsType<StructMemberVariable_List>())
                {
                    VariableType = "List<"+Member.GetType<StructMemberVariable_List>().ListType+">";
                }
                else if(Member.IsType<StructMemberVariable_Bool>())
                {
                    VariableType = "bool";
                }
                else if(Member.IsType<StructMemberVariable_String>())
                {
                    VariableType = "string";
                }
                else if(Member.IsType<StructMemberVariable_Int>())
                {
                    VariableType = "int";
                }
                else if(Member.IsType<StructMemberVariable_Struct>())
                {
                    VariableType = Member.GetType<StructMemberVariable_Struct>().StructType;
                }
                else if(Member.IsType<StructMemberVariable_Raw>())
                {
                    VariableType = Member.GetType<StructMemberVariable_Raw>().RawMemberType;
                }
                else if(Member.IsType<StructMemberVariable_tokenPosition>())
                {
                    VariableType = "MBCC.TokenPosition";
                }
                OutStream << VariableType<<" "<<Member.GetName();
                if(Member.GetDefaultValue().Value != "")
                {
                    OutStream<<" = "<<Member.GetDefaultValue().Value;
                }
                else if(VariableType == "string")
                {
                    OutStream << " = \"\"";   
                }
                else
                {
                    //must default construct stuff
                    OutStream << " = new "<<VariableType<<"()";
                }
                OutStream<<";\n";
            }
            OutStream<<"\n}\n";
        }
    }
    void CSharpParserGenerator::p_WriteGetTokenizer(MBCCDefinitions const& Grammar,MBUtility::MBOctetOutputStream& OutStream)
    {
        OutStream<<"public MBCC.Tokenizer GetTokenizer()\n{\n";
        OutStream<<"return new MBCC.Tokenizer("<<CLikeParser::LiteralEscapeString(Grammar.SkipRegex)<<",";
        for(int i = 0; i < Grammar.Terminals.size();i++)
        {
            OutStream<<CLikeParser::LiteralEscapeString(Grammar.Terminals[i].RegexDefinition);   
            if(i + 1 < Grammar.Terminals.size())
            {
                OutStream<<",";
            }
        }
        OutStream<<");";
        OutStream<<"\n}\n";
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
        SourceIndent<<"using System;\n";
        SourceIndent<<"using System.Collections.Generic;\n";
        SourceIndent<<"namespace Parser\n{\n";
        p_WriteClasses(Grammar,SourceIndent);

        SourceIndent<<"public class Parser\n{\n";
        CLikeParser ParserGenerator;
        ParserGenerator.SetAdapter(this);
        ParserGenerator.WriteNonTerminalFunctions(Grammar,TotalProductions,SourceIndent);
        p_WriteGetTokenizer(Grammar,SourceIndent);
        SourceIndent<<"\n}\n";
        SourceIndent<<"\n}\n";
    }
    std::string CSharpParserGenerator::p_BuiltinToCSharp(TypeInfo Info)
    {
        std::string ReturnValue;       
        if(Info & TypeFlags::Token)
        {
            ReturnValue += "MBCC.Token";   
        }
        else if(Info & TypeFlags::String)
        {
            ReturnValue += "string";
        }
        else if(Info & TypeFlags::TokenPos)
        {
            ReturnValue += "MBCC.TokenPosition";
        }
        else if(Info & TypeFlags::Bool)
        {
            ReturnValue += "bool";
        }
        else if(Info & TypeFlags::Int)
        {
            ReturnValue += "int";
        }
        return(ReturnValue);
    }
    std::string CSharpParserGenerator::GetTypeString(MBCCDefinitions const& Grammar,TypeInfo Type) 
    {
        std::string ReturnValue;
        bool IsList = (Type & TypeFlags::List) != 0;
        Type = Type & (~TypeFlags::List);
        if(IsList)
        {
            ReturnValue = "List<";
        }
        if(Builtin(Type))
        {
            ReturnValue += p_BuiltinToCSharp(Type);
        }
        else
        {
            ReturnValue += Grammar.Structs[Type].Name; 
        }
        if(IsList) ReturnValue += ">"; 
        return(ReturnValue);
    }
    std::string CSharpParserGenerator::GetRHSString(MBCCDefinitions const& Grammar,RuleComponent const& ComponentToInspect) 
    {
        std::string ReturnValue;
        if(ComponentToInspect.IsTerminal)
        {
            if(ComponentToInspect.AssignedMember.IsEmpty())
            {
                return "";   
            }
            if(ComponentToInspect.AssignedMember.ResultType & TypeFlags::String)
            {
                return "Tokenizer.Peek().Value";
            }
            else if(ComponentToInspect.AssignedMember.ResultType & TypeFlags::Bool)
            {
                return "Tokenizer.Peek().Value == \"true\"";
            }
            else if(ComponentToInspect.AssignedMember.ResultType & TypeFlags::Int)
            {
                return "Int32.Parse(Tokenizer.Peek().Value)";
            }
        }
        if(ComponentToInspect.ReferencedRule.IsType<MemberReference>())
        {
            MemberReference const& Expr = ComponentToInspect.ReferencedRule.GetType<MemberReference>();
            if(Expr.Names[0] == "TOKEN")
            {
                return "Tokenizer.Peek()."+Expr.Names[1];
            }
            //no a terminal, and not a special rule, therefore a reference to a non-termial parse function + optional 
            //member references
            ReturnValue = "Parse"+Grammar.NonTerminals[ComponentToInspect.ComponentIndex].Name+"(Tokenizer)";
            for(int i = 1; i < Expr.Names.size();i++)
            {
                ReturnValue += "."+Expr.Names[i];
            }
        }
        else if(ComponentToInspect.ReferencedRule.IsType<Literal>())
        {
            if(ComponentToInspect.ReferencedRule.ResultType & TypeFlags::String)
            {
                ReturnValue = "\""+ComponentToInspect.ReferencedRule.GetType<Literal>().LiteralString+"\"";
            }
            else
            {
                ReturnValue = ComponentToInspect.ReferencedRule.GetType<Literal>().LiteralString;   
            }
        }
        else
        {
            assert(false && "p_GetRHSString doesn't cover all cases for ReferencedRule.GetType");   
        }
        return ReturnValue;
    }
    std::string CSharpParserGenerator::GetLHSMember(MBCCDefinitions const& Grammar,NonTerminal const& AssociatedNonTerminal
    ,ParseRule const& Production,RuleComponent const& ComponentToInspect) 
    {
        std::string MemberAccessString;
        MemberReference const& Expr = ComponentToInspect.AssignedMember.GetType<MemberReference>();
        for(int i = 0; i < Expr.Names.size();i++)
        {
            MemberAccessString  += "."+Expr.Names[i];
        }
        return MemberAccessString;
    }
    std::string CSharpParserGenerator::GetThrowExpectedException(std::string const& CurrentRule,std::string const& ExpectedRule) 
    {
        return "throw new System.Exception(\"Error parsing \" + \""+CurrentRule+"\" + \": Expected \" + \"" +ExpectedRule+"\");\n";
    }
    std::string CSharpParserGenerator::GetFunctionArguments() 
    {
        return "MBCC.Tokenizer Tokenizer";
    }
    std::string CSharpParserGenerator::ListAddFunc() 
    {
        return "Add";
    }
    std::string CSharpParserGenerator::GetLookTableVariable(int TotalProductionSize,int LookDepth,int TerminalCount) 
    {
        return "bool[,,] LOOKTable";
    }
    std::string CSharpParserGenerator::GetLookIndex(std::string First,std::string Second, std::string Third) 
    {
        return "["+First+","+Second+","+Third+"]";
    }
    std::string CSharpParserGenerator::GetFunctionPrefix() 
    {
        return "public ";   
    }
    std::string CSharpParserGenerator::VariableInitializationString(std::string const& TypeString)
    {
        return "new "+TypeString+"()";
    }
}
