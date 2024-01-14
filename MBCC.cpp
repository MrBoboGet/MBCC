#include "MBCC.h"
#include "MBLSP/LSP_Structs.h"
#include "MBUtility/MBErrorHandling.h"
#include <assert.h>
#include <iostream>
#include <regex>
#include <unordered_map>
#include <set>
#include <numeric>
#include <MBParsing/MBParsing.h>


#include "Compilers/Cpp.h"
#include "Compilers/CSharp.h"
#include "Compilers/CppIRParser.h"
namespace MBCC
{
    TypeInfo BuiltinToType(std::string const& BuiltinType)
    {
        TypeInfo ReturnValue = -1;
        if(BuiltinType == "string")
        {
            ReturnValue = TypeFlags::String;   
        }
        else if(BuiltinType == "int")
        {
            ReturnValue = TypeFlags::Int;   
        }
        else if(BuiltinType == "bool")
        {
            ReturnValue = TypeFlags::Bool;   
        }
        else if(BuiltinType == "tokenPos")
        {
            ReturnValue = TypeFlags::TokenPos;   
        }
        else
        {
            assert(false && "invalid builtin type given for h_BuiltinToType");   
        }
        return(ReturnValue);
    }
    //BEGIN StructMemberVariable
    
    StructMemberVariable::StructMemberVariable(StructMemberVariable_List ListMemberVariable)
        : m_Content(std::move(ListMemberVariable))
    {
        int ReturnValue = 0;

    }
    StructMemberVariable::StructMemberVariable(StructMemberVariable_Raw RawMemberVariable)
        : m_Content(std::move(RawMemberVariable))
    {
           
    }
    StructMemberVariable::StructMemberVariable(StructMemberVariable_Struct StructMemberVariable)
        : m_Content(std::move(StructMemberVariable))
    {
           
    }
    StructMemberVariable::StructMemberVariable(StructMemberVariable_Int RawMemberVariable)
        : m_Content(std::move(RawMemberVariable))
    {
           
    }
    StructMemberVariable::StructMemberVariable(StructMemberVariable_String StructMemberVariable)
        : m_Content(std::move(StructMemberVariable))
    {
           
    }
    StructMemberVariable::StructMemberVariable(StructMemberVariable_tokenPosition StructMemberVariable)
        : m_Content(std::move(StructMemberVariable))
    {
        

    }
    StructMemberVariable::StructMemberVariable(StructMemberVariable_Bool StructMemberVariable)
        : m_Content(std::move(StructMemberVariable))
    {
           
    }
    std::string& StructMemberVariable::GetName()
    {
        return(std::visit([&](MemberVariable& x) -> std::string& 
            {
                return(x.Name);      
            }, m_Content));
    }
    Identifier& StructMemberVariable::GetDefaultValue()
    {
        return(std::visit([&](MemberVariable& x) -> Identifier& 
            {
                return(x.DefaultValue);      
            }, m_Content));
    }
    std::string const& StructMemberVariable::GetName() const
    {
        return(std::visit([&](MemberVariable const& x) -> std::string const& 
            {
                return(x.Name);      
            }, m_Content));
    }
    Identifier const& StructMemberVariable::GetDefaultValue() const
    {
        return(std::visit([&](MemberVariable const& x) -> Identifier const& 
            {
                return(x.DefaultValue);      
            }, m_Content));
    }
    void StructMemberVariable::Accept(MemberVariableVisitor& Visitor)
    {
        
    }
    //END StructMemberVariable

    //BEGIN StructDefinition
    //bool StructDefinition::HasMember(std::string const& MemberToCheck) const
    //{
    //    for(auto const& Member : MemberVariables)
    //    {
    //        if(Member.GetName() == MemberToCheck)
    //        {
    //            return(true);  
    //        } 
    //    }       
    //    return(false);
    //}
    //StructMemberVariable const& StructDefinition::GetMember(std::string const& MemberName) const
    //{
    //    for(auto const& Member : MemberVariables)
    //    {
    //        if(Member.GetName() == MemberName)
    //        {
    //            return(Member);
    //        } 
    //    }       
    //    throw std::runtime_error("no member exists with name "+MemberName);
    //}
    //StructMemberVariable& StructDefinition::GetMember(std::string const& MemberName)
    //{
    //    for(auto& Member : MemberVariables)
    //    {
    //        if(Member.GetName() == MemberName)
    //        {
    //            return(Member);  
    //        } 
    //    }       
    //    throw std::runtime_error("no member exists with name "+MemberName);
    //}
    //END StructDefinition
    Identifier MBCCDefinitions::p_ParseIdentifier(const char* Data,size_t DataSize,size_t InParseOffset,size_t* OutParseOffset)
    {
        Identifier ReturnValue;
        size_t ParseOffset = InParseOffset;
        MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        size_t StringBegin = ParseOffset;
        ReturnValue.ByteOffset = StringBegin;
        while(ParseOffset < DataSize)
        {
            if(!((Data[ParseOffset] >= 65 && Data[ParseOffset] <= 90) || (Data[ParseOffset] >= 97 && Data[ParseOffset] <= 122)
                        || Data[ParseOffset] == '_' || (Data[ParseOffset] >= 48 && Data[ParseOffset] <= 57)))
            {
                break; 
            }   
            ParseOffset++;
        }
        ReturnValue.Value = std::string(Data+StringBegin,Data+ParseOffset);
        if(ReturnValue.Value == "")
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: empty identifiers are not allowed",ParseOffset);
        }
        *OutParseOffset = ParseOffset;
        return(ReturnValue);
    }
    std::string h_ParseRegex(const char* Data,size_t DataSize,size_t InParseOffset,size_t* OutParseOffset,MBError* OutError)
    {
        std::string ReturnValue;
        assert(Data[InParseOffset] == '"' && "h_ParseRegex must be called at precise start of the regex string"); 
        //Assume start at "
        size_t  StringBegin = InParseOffset+1;
        size_t ParseOffset = InParseOffset+1; 
        while(ParseOffset < DataSize)
        {
            size_t StringEnd = std::find(Data+ParseOffset,Data+DataSize,'"')-Data;
            if(StringEnd == DataSize)
            {
                *OutError = false;
                OutError->ErrorMessage = "missing unescaped \" at the end of regex string";
                break;
            }
            size_t EscapeBegin = StringEnd-1; 
            int EscapeCount = 0;
            //Guaranteed to not go past InParseOffset[InParseOffset], as it's '"'
            while(Data[EscapeBegin] == '\\')
            {
                EscapeCount += 1; 
                EscapeBegin -= 1;
            }
            if(EscapeCount > 0)
            {
                //EscapeBegin points now at the first \ character
                EscapeBegin += 1;
                //for every even \ add it to the overall string, if even we continue, otherwise we break
                ReturnValue.insert(ReturnValue.end(),Data+ParseOffset,Data+EscapeBegin+(EscapeCount/2));
                if(EscapeCount % 2 == 0)
                {
                    ParseOffset = StringEnd;
                    break;
                }
                else
                {
                    ReturnValue += '"';
                    ParseOffset = StringEnd+1;
                }
            }
            else
            {
                ReturnValue.insert(ReturnValue.end(),Data+ParseOffset,Data+StringEnd);
                ParseOffset = StringEnd;
                break;
            }
        }
        if(ParseOffset == DataSize)
        {
            *OutError = false;
            OutError->ErrorMessage = "missing unescaped \" at the end of regex string";
        }
        *OutParseOffset = ParseOffset+1;
        return(ReturnValue);
    }
    Terminal MBCCDefinitions::p_ParseTerminal(const char* Data,size_t DataSize,size_t InParseOffset,size_t* OutParseOffset,LSPInfo& OutInfo)
    {
        Terminal ReturnValue;  
        size_t ParseOffset = InParseOffset;
        Identifier TerminalIdentifier = p_ParseIdentifier(Data,DataSize,ParseOffset,&ParseOffset);
        OutInfo.SemanticsTokens.push_back(DefinitionsToken(TerminalIdentifier,DefinitionsTokenType::Terminal));
        std::string& TerminalName = TerminalIdentifier.Value;
        if(TerminalName == "")
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: term needs non empty terminal name ",ParseOffset);
        } 
        MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        if(ParseOffset >= DataSize || Data[ParseOffset] != '=')
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: term needs delimiting =",ParseOffset);
        }
        ParseOffset+=1;
        MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        if(ParseOffset >= DataSize || Data[ParseOffset] != '"')
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: term regex must begin with \"",ParseOffset);
        }
        MBError ParseStringResult = true;
        DefinitionsToken NewToken;
        NewToken.ByteOffset = ParseOffset;
        std::string RegexString = h_ParseRegex(Data,DataSize,ParseOffset,&ParseOffset,&ParseStringResult);
        NewToken.Length = ParseOffset-NewToken.ByteOffset;
        NewToken.Type = DefinitionsTokenType::String;
        OutInfo.SemanticsTokens.push_back(NewToken);
        if(!ParseStringResult)
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: error parsing quoted string for terminal definition: "+
                    ParseStringResult.ErrorMessage,ParseOffset);
        }
        if(RegexString == "")
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: term needs non empty regex definition",ParseOffset);
        }
        MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        if(ParseOffset == DataSize || Data[ParseOffset] != ';')
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: no delmiting ; for end of def",ParseOffset);
        }
        ParseOffset+=1;
        *OutParseOffset = ParseOffset;
        ReturnValue.Name = TerminalName;
        ReturnValue.RegexDefinition = RegexString;
        return(ReturnValue);
    }
    StructMemberVariable MBCCDefinitions::p_ParseMemberVariable(const char* Data,size_t DataSize,size_t InParseOffset,size_t* OutParseOffset,LSPInfo& OutInfo)
    {
        StructMemberVariable ReturnValue;
        size_t ParseOffset = InParseOffset;
        MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        if(ParseOffset >= DataSize)
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: end of file before member variable definition or end of struct",ParseOffset);   
        }
        size_t StructBeginOffset = ParseOffset;
        if(Data[ParseOffset] == '{')
        {
            ParseOffset+=1;    
            size_t TypeEnd = std::find(Data+ParseOffset,Data+DataSize,'}')-Data;
            if(TypeEnd >= DataSize)
            {
                throw MBCCParseError("Syntactic error parsing MBCC definitions: Raw member type requries } delimiting the end of the type name",ParseOffset);   
            }
            StructMemberVariable_Raw RawMemberVariable;
            RawMemberVariable.RawMemberType =  std::string(Data+ParseOffset,Data+TypeEnd);
            ParseOffset = TypeEnd+1;
            ReturnValue = StructMemberVariable(RawMemberVariable);
        }
        else
        {
            Identifier StructTypeIdentifier = p_ParseIdentifier(Data,DataSize,ParseOffset,&ParseOffset);
            OutInfo.SemanticsTokens.push_back(DefinitionsToken(StructTypeIdentifier,DefinitionsTokenType::Class));
            std::string& StructType = StructTypeIdentifier.Value;
            MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
            if(ParseOffset >= DataSize)
            {
                throw MBCCParseError("Syntactic error parsing MBCC definitions: member variable needs delimiting ;",ParseOffset);
            }
            if(StructType == "List")
            {
                if(Data[ParseOffset] != '<')
                {
                    throw MBCCParseError("Syntactic error parsing MBCC definitions: builtin type List requires tempalte argument",ParseOffset);
                }
                ParseOffset +=1;
                StructMemberVariable_List List;
                Identifier ListTypeIdentifier = p_ParseIdentifier(Data,DataSize,ParseOffset,&ParseOffset);
                List.ListByteOffset = ListTypeIdentifier.ByteOffset;
                OutInfo.SemanticsTokens.push_back(DefinitionsToken(ListTypeIdentifier,DefinitionsTokenType::Class));
                List.ListType = ListTypeIdentifier.Value;
                MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
                if(ParseOffset >= DataSize || Data[ParseOffset] != '>')
                {
                    throw MBCCParseError("Syntactic error parsing MBCC definitions: builtin type List requires delimiting > for template argument",ParseOffset);
                }
                ParseOffset += 1;
                ReturnValue = StructMemberVariable(List);
            }
            else
            {
                StructMemberVariable_Struct MemberType;
                MemberType.StructType = StructType;
                ReturnValue = StructMemberVariable(MemberType);
            }
        }
        MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        Identifier MemberVariableIdentifier = p_ParseIdentifier(Data,DataSize,ParseOffset,&ParseOffset);
        OutInfo.SemanticsTokens.push_back(DefinitionsToken(MemberVariableIdentifier,DefinitionsTokenType::Variable));
        std::string MemberVariableName = MemberVariableIdentifier.Value;
        ReturnValue.GetName() = MemberVariableName;
        MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        if(ParseOffset >= DataSize)
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: member variable needs delimiting ;",ParseOffset);   
        }
        if(Data[ParseOffset] == ';')
        {
            ParseOffset+=1;   
        }
        else if(Data[ParseOffset] == '=')
        {
            ParseOffset +=1;
            //ReturnValue.GetType<MemberVariable>().DefaultValueByteOffset = ParseOffset;
            //A little bit of a hack, but doesnt require the parsing of any particular data
            size_t ValueEnd = std::find(Data+ParseOffset,Data+DataSize,';')-Data;
            if(ValueEnd >= DataSize)
            {
                throw MBCCParseError("Syntactic error parsing MBCC definitions: member variable needs delimiting ; for default value",ParseOffset);   
            }
            ReturnValue.GetDefaultValue().Value = std::string(Data+ParseOffset,Data+ValueEnd);
            ReturnValue.GetDefaultValue().ByteOffset = ParseOffset;
            ParseOffset = ValueEnd+1; 
        }
        else
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: member variable needs delimiting ;",ParseOffset);   
        }
        *OutParseOffset = ParseOffset;
        //ReturnValue.GetType<MemberVariable>().BeginOffset = StructBeginOffset;
        ReturnValue.Visit([&](MemberVariable& var)
                {
                    var.BeginOffset = StructBeginOffset;
                });
        return(ReturnValue);
    }
    /*
struct Hej1 : Hej2
{
    List<Hej2> Hejs;
    {RawValue} RawTest;
}
    */
    StructDefinition MBCCDefinitions::p_ParseStruct(const char* Data,size_t DataSize,size_t InParseOffset,size_t* OutParseOffset,LSPInfo& OutInfo)
    {
        StructDefinition ReturnValue;
        size_t ParseOffset = InParseOffset; 
        ReturnValue.StructBegin = ParseOffset;
        Identifier NameIdentifier = p_ParseIdentifier(Data,DataSize,ParseOffset,&ParseOffset);
        OutInfo.SemanticsTokens.push_back(DefinitionsToken(NameIdentifier,DefinitionsTokenType::Class));
        ReturnValue.Name = NameIdentifier.Value;
        MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        if(ParseOffset < DataSize && Data[ParseOffset] == ':')
        {
            ParseOffset+=1;
            Identifier ParentIdentifier = p_ParseIdentifier(Data,DataSize,ParseOffset,&ParseOffset);
            OutInfo.SemanticsTokens.push_back(DefinitionsToken(ParentIdentifier,DefinitionsTokenType::Class));
            ReturnValue.ParentStruct = ParentIdentifier.Value;
            ReturnValue.ParentOffset = ParentIdentifier.ByteOffset;
            MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        }
        if(ParseOffset >= DataSize || Data[ParseOffset] != '{')
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: struct needs delimiting { for start of member variables",ParseOffset);   
        }
        ParseOffset+=1;
        MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        bool EndDelimiterFound = false;
        while(ParseOffset < DataSize)
        {
            if(Data[ParseOffset] == '}')
            {
                EndDelimiterFound = true;
                ParseOffset+=1;
                break;
            }      
            ReturnValue.MemberVariables.push_back(p_ParseMemberVariable(Data,DataSize,ParseOffset,&ParseOffset,OutInfo));
            MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        }
        if(!EndDelimiterFound)
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: struct needs delimiting } for end of member variables",ParseOffset);   
        }
        *OutParseOffset = ParseOffset;
        return(ReturnValue); 
    }
    std::pair<Identifier,Identifier> MBCCDefinitions::p_ParseDef(const char* Data,size_t DataSize,size_t InParseOffset,size_t* OutParseOffset,LSPInfo& OutInfo)
    {
        std::pair<Identifier,Identifier> ReturnValue; 
        size_t ParseOffset = InParseOffset;
        Identifier RuleIdentifier = p_ParseIdentifier(Data,DataSize,ParseOffset,&ParseOffset);
        OutInfo.SemanticsTokens.push_back(DefinitionsToken(RuleIdentifier,DefinitionsTokenType::NonTerminal));
        std::string RuleName = RuleIdentifier.Value;
        if(RuleName == "")
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: def needs non empty rule name ",ParseOffset);
        } 
        MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        if(ParseOffset >= DataSize || Data[ParseOffset] != '=')
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: def needs delimiting =",ParseOffset);
        }
        ParseOffset+=1;
        MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        Identifier StructIdentifier = p_ParseIdentifier(Data,DataSize,ParseOffset,&ParseOffset);
        OutInfo.SemanticsTokens.push_back(DefinitionsToken(StructIdentifier,DefinitionsTokenType::Class));
        std::string StructName = StructIdentifier.Value;
        if(StructName == "")
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: def needs non empty struct name ",ParseOffset);
        }
        MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        if(ParseOffset == DataSize || Data[ParseOffset] != ';')
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: no delmiting ; for end of def",ParseOffset);
        }
        ParseOffset+=1;
        *OutParseOffset = ParseOffset;
        ReturnValue.first = RuleIdentifier;
        ReturnValue.second = StructIdentifier;
        return(ReturnValue);
    }
    SemanticAction MBCCDefinitions::p_ParseSemanticAction(const char* Data,size_t DataSize,size_t InParseOffset,size_t* OutParseOffset)
    {
        SemanticAction ReturnValue;
        size_t ParseOffset = InParseOffset;
        if(Data[ParseOffset] != '{')
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: { must begin semantic action",ParseOffset);  
        } 
        ParseOffset+=1;
        size_t ActionEnd = std::find(Data+ParseOffset,Data+DataSize,'}')-Data;
        if(ActionEnd == DataSize)
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: no delimiting } for semantic action",ParseOffset);
        }
        ReturnValue.ActionString = std::string(Data+ParseOffset,Data+ActionEnd);
        ParseOffset = ActionEnd+1;
        *OutParseOffset = ParseOffset;     
        return ReturnValue;       
    }
    //int MBCCDefinitions::p_ParseAssignOrder(const char* Data,size_t DataSize,size_t InParseOffset,size_t* OutParseOffset)
    //{
    //    int ReturnValue = -1;
    //    size_t ParseOffset = InParseOffset;
    //    assert(Data[ParseOffset] == '>');
    //    ParseOffset += 1;
    //    size_t SpacePos = std::find(Data+ParseOffset,Data+DataSize,' ')-Data;
    //    if(SpacePos == DataSize)
    //    {
    //        throw MBCCParseError("Syntactic error parsing order specification: space needed to delimit end of integer",InParseOffset);   
    //    }
    //    std::string IntString = std::string(Data+ParseOffset,Data+SpacePos);
    //    try
    //    {
    //        ReturnValue = std::stoi(IntString);
    //    }
    //    catch(std::exception const& e)
    //    {
    //        throw MBCCParseError("Syntactic error parsing order specification: Invalid integer string: "+IntString,InParseOffset);   
    //    }
    //    if(ReturnValue < 0)
    //    {
    //        throw MBCCParseError("Semantic error parsing order specification: order has to be non-negative integer"+IntString,InParseOffset);   
    //    }
    //    return(ReturnValue);
    //}
    bool h_IsNumber(std::string_view String)
    {
        bool ReturnValue = true;
        for(int i = 0; i < String.size();i++)
        {
            if ( !( (i == 0 && String[0] == '-') || (String[i] >= '0' && String[i] < '9')))
            {
                return false;
            }
        }
        return ReturnValue;
    }
    MemberExpression MBCCDefinitions::p_ParseMemberExpression(const char* Data,size_t DataSize,size_t InParseOffset,size_t* OutParseOffset,bool IsLHS, int& CurrentLambdaID,std::vector<Lambda>& OutLambdas,LSPInfo& OutInfo)
    {
        MemberExpression ReturnValue;
        size_t ParseOffset = InParseOffset;
        MemberReference& MemberExpr = ReturnValue.SetType<MemberReference>();
        MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        while(ParseOffset < DataSize)
        {
            if(Data[ParseOffset] == '"')
            {
                MBError Error = true;
                size_t StringBegin = ParseOffset;
                std::string QuotedString = MBParsing::ParseQuotedString(Data,DataSize,ParseOffset,&ParseOffset,&Error);
                if(!Error)
                {
                    throw  MBCCParseError("Error parsing quoted string: "+Error.ErrorMessage,ParseOffset);   
                }
                ReturnValue.SetType<Literal>().LiteralString = QuotedString;
                ReturnValue.ResultType = TypeFlags::String;
                OutInfo.SemanticsTokens.push_back(DefinitionsToken(StringBegin,QuotedString.size()+2,DefinitionsTokenType::String));
                break;
            }

            Identifier PartIdentifier = p_ParseIdentifier(Data,DataSize,ParseOffset,&ParseOffset);

            if(PartIdentifier.Value == "true" || PartIdentifier.Value == "false")
            {
                ReturnValue.ResultType = TypeFlags::Bool;
                ReturnValue.SetType<Literal>().LiteralString = PartIdentifier.Value;
                OutInfo.SemanticsTokens.push_back(DefinitionsToken(PartIdentifier,DefinitionsTokenType::Bool));
                break;
            }
            if(h_IsNumber(PartIdentifier.Value))
            {   
                ReturnValue.ResultType = TypeFlags::Int;
                ReturnValue.SetType<Literal>().LiteralString = PartIdentifier.Value;
                OutInfo.SemanticsTokens.push_back(DefinitionsToken(PartIdentifier,DefinitionsTokenType::Number));
                break;
            }

            std::string NewPart = PartIdentifier.Value;
            MemberExpr.Names.push_back(std::move(NewPart));
            MemberExpr.PartTypes.push_back(-1);
            MemberExpr.PartByteOffsets.push_back(PartIdentifier.ByteOffset);

            if(ParseOffset < DataSize && Data[ParseOffset] == '(')
            {
                if(IsLHS)
                {
                    break;   
                }
                //in potential lambda declaration
                if(MemberExpr.Names.size() != 1)
                {
                    throw MBCCParseError("Syntactic error parsing MBCC definitions: unexpected ( in MemberExpression",ParseOffset);
                }
                //first string is the type of the lambda, the rest is the actual lambda
                Lambda NewLambda;
                NewLambda.Type.Value = MemberExpr.Names[0];
                NewLambda.Type.ByteOffset = MemberExpr.PartByteOffsets[0];

                OutInfo.SemanticsTokens.push_back(DefinitionsToken(NewLambda.Type,DefinitionsTokenType::Class));
                NewLambda.Name = p_LambdaIDToLambdaName(CurrentLambdaID);
                CurrentLambdaID++;
                ParseOffset += 1;
                NewLambda.Rules = p_ParseParseRules(Data,DataSize,ParseOffset,&ParseOffset,')',CurrentLambdaID,OutLambdas,OutInfo);

                MemberExpr.Names[0] = NewLambda.Name;
                MemberExpr.PartByteOffsets[0] = 0;
                OutLambdas.push_back(NewLambda);
                MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
            }
            MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);

            if(ParseOffset < DataSize && Data[ParseOffset] == '.')
            {
                ParseOffset+=1; 
            }
            else if(ParseOffset < DataSize && Data[ParseOffset] == '(')
            {
                break;   
            }
            else
            {
                break;
            }
        }
        if(ReturnValue.IsType<MemberReference>() && ReturnValue.GetType<MemberReference>().Names.size() == 0)
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: Member expression needs atleast one member",ParseOffset);
        }
        *OutParseOffset = ParseOffset;
        return(ReturnValue);
    }
    std::string MBCCDefinitions::p_LambdaIDToLambdaName(int ID)
    {
        return "_L"+std::to_string(ID);
    }
    std::vector<ParseRule> MBCCDefinitions::p_ParseParseRules(const char* Data,size_t DataSize,size_t InParseOffset,size_t* OutParseOffset,char EndMarker,int& CurrentLambdaID,std::vector<Lambda>& OutLambdas,LSPInfo& OutInfo)
    {
        std::vector<ParseRule> ReturnValue;
        size_t ParseOffset = InParseOffset;
        ParseRule CurrentRule;
        int TotalComponents = 0;
        MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        while(ParseOffset < DataSize)
        {
            //does allow for empty rules, s
            if(Data[ParseOffset] == '{')
            {
                CurrentRule.Actions.push_back(p_ParseSemanticAction(Data,DataSize,ParseOffset,&ParseOffset));
                MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
                continue;
            }
            if(Data[ParseOffset] == '|')
            {
                if(CurrentRule.Components.size() == 0)
                {
                    throw MBCCParseError("Syntactic error parsing MBCC definitions: emtpy rule is not allowed, | without corresponding components",ParseOffset);
                }
                ReturnValue.push_back(std::move(CurrentRule));          
                CurrentRule = ParseRule();
                TotalComponents = 0;
                ParseOffset+=1;
                MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
                continue;
            }
            else if(Data[ParseOffset] == '(')
            {
                ParseOffset += 1;
                std::vector<ParseRule> Rules = p_ParseParseRules(Data,DataSize,ParseOffset,&ParseOffset,')',CurrentLambdaID,OutLambdas,OutInfo);
                Lambda NewLambda;
                NewLambda.Name = p_LambdaIDToLambdaName(CurrentLambdaID);
                CurrentLambdaID++;
                NewLambda.Rules = std::move(Rules);


                RuleComponent NewComponent;
                MemberReference& ReferencedRule = NewComponent.ReferencedRule.SetType<MemberReference>();

                ReferencedRule.PartTypes.push_back(-1);
                ReferencedRule.Names.push_back(NewLambda.Name);
                ReferencedRule.PartByteOffsets.push_back(-1);
                NewComponent.IsInline = true;
                if(ParseOffset < DataSize)
                {
                    if(Data[ParseOffset] == '+')
                    {
                        NewComponent.Min = 1;    
                        NewComponent.Max = -1;    
                        ParseOffset++;
                    }
                    else if(Data[ParseOffset] == '?')
                    {
                        NewComponent.Min = 0;
                        NewComponent.Max = 1;
                        ParseOffset++;
                    }
                    else if(Data[ParseOffset] == '*')
                    {
                        NewComponent.Min = 0;
                        NewComponent.Max = -1;
                        ParseOffset++;
                    }
                }

                CurrentRule.Components.push_back(std::move(NewComponent));
                OutLambdas.push_back(std::move(NewLambda));
                MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
                continue;
            }
            else if(Data[ParseOffset] == EndMarker)
            {
                ParseOffset+=1;
                break;
            }
            RuleComponent NewComponent;
            //if(Data[ParseOffset] == '>')
            //{
            //    NewComponent.AssignOrder = p_ParseAssignOrder(Data,DataSize,ParseOffset,&ParseOffset);
            //    MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
            //}
            size_t PreviousLambdaCount = OutLambdas.size();
            MemberExpression RuleExpression = p_ParseMemberExpression(Data,DataSize,ParseOffset,&ParseOffset,true,CurrentLambdaID,OutLambdas,OutInfo);    
            MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
            if(ParseOffset >= DataSize)
            {
                throw MBCCParseError("Syntactic error parsing MBCC definitions: missing ; in rule definition",ParseOffset);
            }
            if(Data[ParseOffset] == '=')
            {
                //member assignment    
                ParseOffset += 1;
                NewComponent.AssignedMember = std::move(RuleExpression);
                if(PreviousLambdaCount < OutLambdas.size())
                {
                    throw MBCCParseError("Syntactic error parsing MBCC definitions: lambda only allowed as the right hand side in an assignment",ParseOffset);
                }
                RuleExpression = p_ParseMemberExpression(Data,DataSize,ParseOffset,&ParseOffset,false,CurrentLambdaID,OutLambdas,OutInfo);
                MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
            }
            else
            {
                if(RuleExpression.IsType<Literal>())
                {
                    throw MBCCParseError("Syntactic error parsing MBCC definitions: Literal only allowed in assignment",ParseOffset);
                }
                if(RuleExpression.IsType<MemberReference>() && RuleExpression.GetType<MemberReference>().Names.size() > 1)
                {
                    throw MBCCParseError("Syntactic error parsing MBCC definitions: Rule member expression only allowed in member assignment",ParseOffset);
                }    
            }
            if(RuleExpression.IsType<MemberReference>())
            {
                MemberReference& Expr = RuleExpression.GetType<MemberReference>();
                for(int i = 0; i < Expr.Names.size();i++)
                {
                    if(i == 0)
                    {
                        DefinitionsToken NewToken;
                        NewToken.ByteOffset = Expr.PartByteOffsets[i];
                        NewToken.Length = Expr.Names[i].size();
                        NewToken.Type = DefinitionsTokenType::Rule;
                        OutInfo.SemanticsTokens.push_back(NewToken);
                    }    
                    else
                    {
                        DefinitionsToken NewToken;
                        NewToken.ByteOffset = Expr.PartByteOffsets[i];
                        NewToken.Length = Expr.Names[i].size();
                        NewToken.Type = DefinitionsTokenType::AssignedRHS;
                        OutInfo.SemanticsTokens.push_back(NewToken);
                    }
                }
            }
            if(NewComponent.AssignedMember.IsType<MemberReference>())
            {
                MemberReference& Expr = NewComponent.AssignedMember.GetType<MemberReference>();
                for(int i = 0; i < Expr.Names.size();i++)
                {
                    DefinitionsToken NewToken;
                    NewToken.ByteOffset = Expr.PartByteOffsets[i];
                    NewToken.Length = Expr.Names[i].size();
                    NewToken.Type = DefinitionsTokenType::AssignedLHS;
                    OutInfo.SemanticsTokens.push_back(NewToken);
                }
            }
            if(ParseOffset >= DataSize)
            {
                throw MBCCParseError("Syntactic error parsing MBCC definitions: missing ; in rule definition",ParseOffset);
            }
            if(Data[ParseOffset] == '+')
            {
                NewComponent.Min = 1;    
                NewComponent.Max = -1;    
                ParseOffset++;
            }
            else if(Data[ParseOffset] == '?')
            {
                NewComponent.Min = 0;
                NewComponent.Max = 1;
                ParseOffset++;
            }
            else if(Data[ParseOffset] == '*')
            {
                NewComponent.Min = 0;
                NewComponent.Max = -1;
                ParseOffset++;
            }
            NewComponent.ReferencedRule = std::move(RuleExpression);
            if(NewComponent.ReferencedRule.IsType<MemberReference>() && NewComponent.ReferencedRule.GetType<MemberReference>().Names[0] == "TOKEN")
            {
                CurrentRule.MetaComponents.push_back({ CurrentRule.Components.size() + CurrentRule.MetaComponents.size() ,std::move(NewComponent)});
            }
            else
            {
                CurrentRule.Components.push_back(std::move(NewComponent));
            }
            TotalComponents += 1;
            MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        }
        if(CurrentRule.Components.size() != 0)
        {
            ReturnValue.push_back(std::move(CurrentRule));   
        }
        if(ReturnValue.size() == 0)
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: empty rule",ParseOffset);   
        }
        *OutParseOffset = ParseOffset;
        return(ReturnValue);
    }
    bool TypeIsBuiltin(std::string const& TypeToVerify)
    {
        return(TypeToVerify == "string" || TypeToVerify == "int" || TypeToVerify == "bool" || TypeToVerify == "tokenPos");
    }
    void MBCCDefinitions::p_VerifyStructs(LSPInfo& OutInfo)
    {
        for(auto& Struct : Structs)
        {
            //Verify that the parent struct actually exists    
            if(Struct.ParentStruct != "" && NameToStruct.find(Struct.Name) == NameToStruct.end())
            {
                OutInfo.Diagnostics.push_back(Diagnostic(Struct.ParentOffset,Struct.ParentStruct.size(),
                            "struct named \""+Struct.Name+"\" is the child of a non existing struct named \""+Struct.ParentStruct+"\""));
            } 
            for(auto& MemberVariable : Struct.MemberVariables)
            {
                if(MemberVariable.IsType<StructMemberVariable_List>())
                {
                    if(!TypeIsBuiltin(MemberVariable.GetType<StructMemberVariable_List>().ListType) && NameToStruct.find(MemberVariable.GetType<StructMemberVariable_List>().ListType) == NameToStruct.end())
                    {
                        OutInfo.Diagnostics.push_back(Diagnostic(MemberVariable.GetType<StructMemberVariable_List>().ListByteOffset,
                                    MemberVariable.GetType<StructMemberVariable_List>().ListType.size(),
                                    "List template value in struct \""+Struct.Name+ "\"references unknowns struct named \""+
                        MemberVariable.GetType<StructMemberVariable_List>().ListType+"\""));
                    }
                }
                else if(MemberVariable.IsType<StructMemberVariable_Struct>())
                {
                    StructMemberVariable_Struct& StructMember = MemberVariable.GetType<StructMemberVariable_Struct>();
                    if(TypeIsBuiltin(MemberVariable.GetType<StructMemberVariable_Struct>().StructType))
                    {
                        if(StructMember.StructType == "int")
                        {
                            StructMemberVariable_Int NewMember;
                            NewMember.Name = MemberVariable.GetName();
                            NewMember.DefaultValue = MemberVariable.GetDefaultValue();
                            NewMember.BeginOffset = MemberVariable.GetBase().BeginOffset;
                            if(NewMember.DefaultValue.Value != "")
                            {
                                try
                                {
                                    NewMember.Value = std::stoi(NewMember.DefaultValue.Value);
                                    OutInfo.SemanticsTokens.push_back(DefinitionsToken(NewMember.DefaultValue,DefinitionsTokenType::Number));
                                }
                                catch(std::exception const& e)
                                {
                                    OutInfo.Diagnostics.push_back(Diagnostic(NewMember.DefaultValue,
                                              "Int member variable not a valid integer"));
                                }
                            }
                            MemberVariable = StructMemberVariable(NewMember);
                        }
                        else if(StructMember.StructType == "string")
                        {
                            StructMemberVariable_String NewMember;
                            NewMember.Value = MemberVariable.GetDefaultValue().Value;
                            NewMember.DefaultValue = MemberVariable.GetDefaultValue();
                            NewMember.Name = MemberVariable.GetName();
                            NewMember.BeginOffset = MemberVariable.GetBase().BeginOffset;
                            MemberVariable = StructMemberVariable(NewMember);
                            OutInfo.SemanticsTokens.push_back(DefinitionsToken(NewMember.DefaultValue,DefinitionsTokenType::String));
                        }
                        else if(StructMember.StructType == "bool")
                        {
                            StructMemberVariable_Bool NewMember;   
                            NewMember.DefaultValue = MemberVariable.GetDefaultValue();
                            NewMember.Name = MemberVariable.GetName();
                            NewMember.BeginOffset = MemberVariable.GetBase().BeginOffset;
                            if(NewMember.DefaultValue.Value != "")
                            {
                                size_t Offset = 0;
                                MBParsing::SkipWhitespace(NewMember.DefaultValue.Value,0,&Offset);
                                if(NewMember.DefaultValue.Value.size() -Offset == 4 && std::memcmp(NewMember.DefaultValue.Value.data()+Offset,"true",4) == 0)
                                {
                                    NewMember.Value = true;   
                                }
                                else if(NewMember.DefaultValue.Value.size() -Offset == 5 && std::memcmp(NewMember.DefaultValue.Value.data()+Offset,"false",5) == 0)
                                {
                                    NewMember.Value = false;   
                                }
                                else
                                {
                                    OutInfo.Diagnostics.push_back(Diagnostic(NewMember.DefaultValue,
                                              "invalid default value for bool type: "+NewMember.DefaultValue.Value));
                                }
                                OutInfo.SemanticsTokens.push_back(DefinitionsToken(NewMember.DefaultValue,DefinitionsTokenType::Bool));
                            }
                            MemberVariable = StructMemberVariable(NewMember);
                        }
                        else if(StructMember.StructType == "tokenPos")
                        {
                            StructMemberVariable_tokenPosition NewMember;   
                            NewMember.DefaultValue = MemberVariable.GetDefaultValue();
                            NewMember.Name = MemberVariable.GetName();
                            NewMember.BeginOffset = MemberVariable.GetBase().BeginOffset;
                            if(NewMember.DefaultValue.Value != "")
                            {
                                OutInfo.Diagnostics.push_back(Diagnostic(NewMember.DefaultValue,
                                              "tokenPos cannot have a default type"));
                            }
                            MemberVariable = std::move(NewMember);
                        }
                    }
                    else if(NameToStruct.find(MemberVariable.GetType<StructMemberVariable_Struct>().StructType) == NameToStruct.end())
                    {
                        OutInfo.Diagnostics.push_back(Diagnostic(
                                      MemberVariable.GetType<StructMemberVariable_Struct>().BeginOffset,
                                      MemberVariable.GetType<StructMemberVariable_Struct>().StructType.size(),
                                      "member variable in struct \""+Struct.Name+"\" refernces unknowns struct named \""+
                                MemberVariable.GetType<StructMemberVariable_Struct>().StructType+"\""));
                    }
                }
            }
        }
        if(OutInfo.Diagnostics.size() == 0)
        {
            DepInfo = CalculateDependancyInfo(*this);
        }
    }
    //Assumption: Structs are valid
    bool MBCCDefinitions::HasMember(StructDefinition const& StructDef,std::string const& MemberName)
    {
        bool ReturnValue = false;
        for(auto const& Member : StructDef.MemberVariables)
        {
            if(Member.GetName() == MemberName)
            {
                ReturnValue = true;
                break;
            }
        }       
        if(!ReturnValue && StructDef.ParentStruct != "")
        {
            assert(NameToStruct.find(StructDef.ParentStruct) != NameToStruct.end());
            ReturnValue = HasMember(Structs[NameToStruct[StructDef.ParentStruct]],MemberName);
        }
        return(ReturnValue);
    }
    StructMemberVariable const* MBCCDefinitions::TryGetMember(StructDefinition const& StructDef,std::string const& MemberName) const
    {
        StructMemberVariable const* ReturnValue = nullptr;      
        for(auto const& Member : StructDef.MemberVariables)
        {
            if(Member.GetName() == MemberName)
            {
                return(&Member);
            }
        }
        if(StructDef.ParentStruct == "")
        {
            return nullptr;
        }    
        return(TryGetMember(Structs[NameToStruct.at(StructDef.ParentStruct)],MemberName));
    }
    StructMemberVariable const& MBCCDefinitions::GetMember(StructDefinition const& StructDef,std::string const& MemberName) const
    {
        for(auto const& Member : StructDef.MemberVariables)
        {
            if(Member.GetName() == MemberName)
            {
                return(Member);
            }
        }       
        if(StructDef.ParentStruct == "")
        {
            throw std::runtime_error("Internal error verifying MBCC definitions: Structdefinition has no member with name \""+MemberName+"\"");   
        }
        assert(NameToStruct.find(StructDef.ParentStruct) != NameToStruct.end());
        return(GetMember(Structs[NameToStruct.at(StructDef.ParentStruct)],MemberName));
    }
    StructMemberVariable& MBCCDefinitions::GetMember(StructDefinition& StructDef,std::string const& MemberName)
    {
        for(auto& Member : StructDef.MemberVariables)
        {
            if(Member.GetName() == MemberName)
            {
                return(Member);
            }
        }       
        if(StructDef.ParentStruct == "")
        {
            throw std::runtime_error("Internal error verifying MBCC definitions: Structdefinition has no member with name \""+MemberName+"\"");   
        }
        assert(NameToStruct.find(StructDef.ParentStruct) != NameToStruct.end());
        return(GetMember(Structs[NameToStruct[StructDef.ParentStruct]],MemberName));
    }
    //Assumes the existance of the types
    bool Builtin(TypeInfo Info)
    {
        bool ReturnValue = false;        
        if(Info >= TypeFlags::Raw)
        {
            ReturnValue = true;   
        }
        return(ReturnValue);
    }
    bool MBCCDefinitions::p_IsAssignable(StructIndex Lhs,StructIndex Rhs)
    {
        bool ReturnValue = false;
        StructIndex CurrentIndex = Rhs;
        if(Builtin(Lhs) && Builtin(Rhs))
        {
            if(Rhs == TypeFlags::String)
            {
                return(true);   
            }
            return(Lhs == Rhs);
        }
        if(Builtin(Lhs) || Builtin(Rhs))
        {
            return(false);   
        }
        while(true)
        {
            if(CurrentIndex == Lhs)
            {
                ReturnValue = true;
                break;   
            }
            if(Structs[CurrentIndex].ParentStruct == "")
            {
                break;   
            }
            CurrentIndex = NameToStruct[Structs[CurrentIndex].ParentStruct];
        }
        return(ReturnValue);       
    }
    std::string h_GetPrintTypeName(MBCCDefinitions const& Grammar,TypeInfo Info)
    {
        std::string ReturnValue;       
        if(Info & TypeFlags::List)
        {
            ReturnValue = "List<";   
        }
        if(Info & TypeFlags::Token)
        {
            ReturnValue += "token";   
        }
        else if(Info & TypeFlags::String)
        {
            ReturnValue += "string";
        }
        else if(Info & TypeFlags::Bool)
        {
            ReturnValue += "bool";
        }
        else if(Info & TypeFlags::Int)
        {
            ReturnValue += "int";
        }
        else if (Info & TypeFlags::TokenPos)
        {
            ReturnValue += "tokenPos";
        }
        else
        {
            ReturnValue += Grammar.Structs[Info & (~TypeFlags::List)].Name;
        }
        if(Info & TypeFlags::List)
        {
            ReturnValue += ">";   
        }
        return(ReturnValue);
    }
    TypeInfo h_ResolveMemberTypeInfo(TypeInfo InitialType,StructDefinition const* CurrentStruct,MemberExpression& Member,size_t NameOffset,LSPInfo& OutInfo,MBCCDefinitions const& Grammar)
    {
        TypeInfo ReturnValue = InitialType;
        if(Member.IsType<Literal>())
        {
            return Member.ResultType;
        }
        MemberReference& Expr = Member.GetType<MemberReference>();
        while(NameOffset < Expr.Names.size())
        {
            if(Builtin(ReturnValue))
            {
                //OutError = "Struct "+h_GetPrintTypeName(Grammar,ReturnValue)+" has no member "+Expr.Names[NameOffset];        
                OutInfo.Diagnostics.push_back(Diagnostic(Expr.PartByteOffsets[NameOffset],Expr.Names[NameOffset].size(),
                            "Struct "+h_GetPrintTypeName(Grammar,ReturnValue)+" has no member "+Expr.Names[NameOffset]));
                ReturnValue = -1;
                break;
            }
            StructMemberVariable const* CurrentMember = Grammar.TryGetMember(*CurrentStruct, Expr.Names[NameOffset]);
            if(CurrentMember == nullptr)
            {
                //OutError = "Struct "+CurrentStruct->Name+" has no member "+Expr.Names[NameOffset];
                OutInfo.Diagnostics.push_back(Diagnostic(Expr.PartByteOffsets[NameOffset],Expr.Names[NameOffset].size(),
                            "Struct "+CurrentStruct->Name+" has no member "+Expr.Names[NameOffset]));
                ReturnValue = -1;
                break;
            }
            //Assumes that structs have been verified
            std::string StructName;
            if(CurrentMember->IsType<StructMemberVariable_String>())
            {
                ReturnValue = TypeFlags::String;
            }
            else if(CurrentMember->IsType<StructMemberVariable_Int>())
            {
                ReturnValue = TypeFlags::Int;
            }
            else if(CurrentMember->IsType<StructMemberVariable_Bool>())
            {
                ReturnValue = TypeFlags::Bool;
            }
            else if(CurrentMember->IsType<StructMemberVariable_Raw>())
            {
                ReturnValue = TypeFlags::Raw;
            }
            else if(CurrentMember->IsType<StructMemberVariable_tokenPosition>())
            {
                ReturnValue = TypeFlags::TokenPos;
            }
            else if(CurrentMember->IsType<StructMemberVariable_Struct>())
            {
                ReturnValue = Grammar.NameToStruct.at(CurrentMember->GetType<StructMemberVariable_Struct>().StructType);
                CurrentStruct = &Grammar.Structs[ReturnValue];
            }
            else if(CurrentMember->IsType<StructMemberVariable_List>())
            {
                std::string const& ListType = CurrentMember->GetType<StructMemberVariable_List>().ListType;
                if(!TypeIsBuiltin(ListType))
                {
                    ReturnValue = Grammar.NameToStruct.at(ListType);
                }
                else
                {
                    ReturnValue = BuiltinToType(ListType);
                }
                ReturnValue |= TypeFlags::List;
            }
            Expr.PartTypes[NameOffset] = ReturnValue;
            NameOffset++;
        }

        return(ReturnValue);
    }
    TypeInfo MBCCDefinitions::p_GetMemberTypeInfo(StructDefinition const& StructScope,MemberExpression& Member,LSPInfo& OutInfo)
    {
        TypeInfo ReturnValue = 0;        
        StructDefinition const* CurrentScope = &StructScope;
        size_t NameOffset = 0;
        if(Member.GetType<MemberReference>().Names[0] == "this")
        {
            StructIndex Index = NameToStruct[CurrentScope->Name];
            //current struct always guaranteed to exist
            CurrentScope = &Structs[Index];
            Member.GetType<MemberReference>().PartTypes[0] = Index;
            NameOffset+=1;
            ReturnValue = Index;
        }
        //ReturnValue = NameToStruct[CurrentScope->Name];
        ReturnValue = h_ResolveMemberTypeInfo(ReturnValue,CurrentScope,Member,NameOffset,OutInfo,*this);
        return(ReturnValue);
    }
    TypeInfo MBCCDefinitions::p_GetRHSTypeInfo(MemberExpression& RHSExpression,int RHSMax,LSPInfo& OutInfo)
    {
        TypeInfo ReturnValue = -1;
        if(RHSExpression.IsType<Literal>())
        {
            return RHSExpression.ResultType;   
        }
        size_t NameOffset = 0; 
        StructDefinition TokenStruct;
        TokenStruct.Name = "Token";
        StructMemberVariable Position = StructMemberVariable_tokenPosition();
        TokenStruct.MemberVariables.push_back(std::move(Position));
        StructDefinition const* CurrentScope = nullptr;
        MemberReference& Expr = RHSExpression.GetType<MemberReference>();
        if(Expr.Names[0] == "TOKEN")
        {
            //can cover all cases here, slightly uggly
            CurrentScope = &TokenStruct;
            Expr.PartTypes[0] = TypeFlags::Token;
            if(Expr.Names.size() == 1)
            {
                //OutError = "Error in evaluating type of RHS: need to select member of TOKEN, TOKEN isn't a valid struct by itself";
                OutInfo.Diagnostics.push_back(Diagnostic(Expr.PartByteOffsets[NameOffset],Expr.Names[NameOffset].size(),
                            "Error in evaluating type of RHS: need to select member of TOKEN, TOKEN isn't a valid struct by itself"));
                return(-1);
            }
            if(Expr.Names.size() > 2)
            {
                //OutError = "Error in evaluating type of RHS: \""+Expr.Names[2]+"\" isn't a valid member";
                OutInfo.Diagnostics.push_back(Diagnostic(Expr.PartByteOffsets[NameOffset],Expr.Names[NameOffset].size(),
                                "Error in evaluating type of RHS: \""+Expr.Names[2]+"\" isn't a valid member"));
                return(-1);
            }
            if(Expr.Names[1] == "Position")
            {
                ReturnValue = TypeFlags::TokenPos;
                Expr.PartTypes[1] = TypeFlags::TokenPos;
                ReturnValue = TypeFlags::TokenPos;
            }
            else
            {
                //OutError = "Error in evaluating type of RHS: TOKEN has no member \""+Expr.Names[1]+"\"";
                OutInfo.Diagnostics.push_back(Diagnostic(Expr.PartByteOffsets[NameOffset],Expr.Names[NameOffset].size(),
                                "Error in evaluating type of RHS: TOKEN has no member \""+Expr.Names[1]+"\""));
                return(-1);
            }
            NameOffset = 2;
        }
        else
        {
            if(Expr.Names.size() == 1)
            {
                auto TermIt = NameToTerminal.find(Expr.Names[0]);
                if(TermIt != NameToTerminal.end())
                {
                    ReturnValue = TypeFlags::String;
                }
            }
            if(ReturnValue == -1)
            {
                auto NonTermIt = NameToNonTerminal.find(Expr.Names[0]);   
                if(NonTermIt == NameToNonTerminal.end())
                {
                    //OutError = "\""+Expr.Names[0]+"\" in right hand isn't a valid nonterminal";
                    OutInfo.Diagnostics.push_back(Diagnostic(Expr.PartByteOffsets[NameOffset],Expr.Names[NameOffset].size(),
                                    "\""+Expr.Names[0]+"\" in right hand isn't a valid nonterminal"));
                    return(-1);
                }
                if(NonTerminals[NonTermIt->second].AssociatedStruct == -1)
                {
                    //OutError = "\""+Expr.Names[0]+"\" in right hand doesn't have an associated struct, and can't be assigned";
                    OutInfo.Diagnostics.push_back(Diagnostic(Expr.PartByteOffsets[NameOffset],Expr.Names[NameOffset].size(),
                                "\""+Expr.Names[0]+"\" in right hand doesn't have an associated struct, and can't be assigned"));
                    return(-1);
                }
                ReturnValue = NonTerminals[NonTermIt->second].AssociatedStruct;
                CurrentScope = &Structs[NonTerminals[NonTermIt->second].AssociatedStruct];
            }
            Expr.PartTypes[0] = ReturnValue;
            NameOffset += 1;
        }
        ReturnValue = h_ResolveMemberTypeInfo(ReturnValue,CurrentScope,RHSExpression,NameOffset,OutInfo,*this);
        if(RHSMax == -1)
        {
            if(ReturnValue & TypeFlags::List)
            {
                //OutError = "Error in evaluating type of RHS: can only apply * to a type not of list";
                OutInfo.Diagnostics.push_back(Diagnostic(Expr.PartByteOffsets[NameOffset],Expr.Names[NameOffset].size(),
                                "Error in evaluating type of RHS: can only apply * to a type not of list"));
                return(-1);
            } 
            ReturnValue |= TypeFlags::List;
        }
        return(ReturnValue);
    }
    bool MBCCDefinitions::p_IsAssignable(StructDefinition const& StructScope,MemberExpression& StructExpression,MemberExpression& RHSExpression,int RHSMax,LSPInfo& OutInfo)
    {
        bool ReturnValue = true;
        TypeInfo LHSType = p_GetMemberTypeInfo(StructScope,StructExpression,OutInfo);
        if(LHSType == -1)
        {
            return(false);   
        }
        StructExpression.ResultType = LHSType;
        TypeInfo RHSType = p_GetRHSTypeInfo(RHSExpression,RHSMax,OutInfo);
        if(RHSType == -1)
        {
            return(false);   
        }

        RHSExpression.ResultType = RHSType;
        StructIndex LHSStruct = LHSType & (~TypeFlags::List);
        StructIndex RHSStruct = RHSType & (~TypeFlags::List);
        if((  ((RHSType & TypeFlags::List) && !(LHSType & TypeFlags::List)) || !p_IsAssignable(LHSStruct,RHSStruct)))
        {
            std::string LHSName = h_GetPrintTypeName(*this,LHSType);
            std::string RHSName = h_GetPrintTypeName(*this,RHSType);
            OutInfo.Diagnostics.push_back(Diagnostic(StructExpression.GetType<MemberReference>().PartByteOffsets[0],StructExpression.GetType<MemberReference>().Names[0].size(),"LHS has type "+LHSName+" and RHS has incompatible type "+RHSName));
            //OutError  = "LHS has type "+LHSName+" and RHS has incompatible type "+RHSName;
            ReturnValue = false;
        }

        return(ReturnValue);
    }
    void MBCCDefinitions::p_VerifyRules(LSPInfo& OutInfo)
    {
        for(auto& NonTerminal : NonTerminals)
        {
            StructDefinition* AssociatedStruct = nullptr;
            if(NonTerminal.AssociatedStruct != -1)
            {
                AssociatedStruct = &Structs[NonTerminal.AssociatedStruct];
            }
            //REFACTOR
            for(auto& Rule : NonTerminal.Rules)
            {
                bool HasThisAssignment = false;
                bool HasRegularAssignment = false;
                for(auto& Component : Rule.Components)
                {
                    p_VerifyComponent(Component,NonTerminal.Name,AssociatedStruct,HasThisAssignment,HasRegularAssignment,OutInfo);
                }
                for (auto& Component : Rule.MetaComponents)
                {
                    p_VerifyComponent(Component.second, NonTerminal.Name, AssociatedStruct, HasThisAssignment, HasRegularAssignment,OutInfo);
                }
                if (HasRegularAssignment && HasThisAssignment)
                {
                    Rule.NeedsAssignmentOrder = true;
                }
            }
        } 
    }
    void MBCCDefinitions::p_VerifyComponent(RuleComponent& Component,std::string const& NonTermName,StructDefinition const* AssociatedStruct,bool& HasThisAssignment,bool& HasRegularAssignment,LSPInfo& OutInfo)
    {
        if(Component.ReferencedRule.IsType<MemberReference>())
        {
            MemberReference& Rhs = Component.ReferencedRule.GetType<MemberReference>();
            if(Rhs.Names[0] == "TOKEN")
            {
                Component.IsTerminal = true;
            }
            else if(auto TermIt = NameToTerminal.find(Rhs.Names[0]); TermIt != NameToTerminal.end())
            {
                Component.IsTerminal = true;
                Component.ComponentIndex = TermIt->second;
                if(Rhs.Names.size() > 1)
                {
                    OutInfo.Diagnostics.push_back(Diagnostic(Rhs.PartByteOffsets[0],Rhs.Names[0].size(),
                                "terminal \""+Rhs.Names[0] + "\" has no member "
                                " \""+Rhs.Names[1]+"\""));
                }
            }   
            else if(auto NonTermIt = NameToNonTerminal.find(Rhs.Names[0]); NonTermIt != NameToNonTerminal.end())
            {
                Component.IsTerminal = false;
                Component.ComponentIndex = NonTermIt->second;
                if(!Component.AssignedMember.IsEmpty())
                {
                    if(NonTerminals[NonTermIt->second].AssociatedStruct == -1)
                    {
                        OutInfo.Diagnostics.push_back(Diagnostic(Rhs.PartByteOffsets[0],Rhs.Names[0].size(),
                                    "assignment in non-terminal"+NonTermName +" but non-terminal doesn't have an associated struct"));
                    }
                }
            }
            else
            {
                OutInfo.Diagnostics.push_back(Diagnostic(Rhs.PartByteOffsets[0],Rhs.Names[0].size(),
                            "rule referencing unkown terminal/non-terminal named"
                            " \""+Rhs.Names[0]+"\""));
            }
        }
        if(OutInfo.Diagnostics.size() != 0)
        {
            return;
        }
        if(AssociatedStruct != nullptr)
        {
            if(Component.AssignedMember.IsEmpty())
            {
                return;   
            }
            if(Component.AssignedMember.GetType<MemberReference>().Names[0] == "this")
            {
                HasThisAssignment = true;
            }
            else
            {
                HasRegularAssignment = true;   
            }
            p_IsAssignable(*AssociatedStruct,Component.AssignedMember,Component.ReferencedRule,Component.Max,OutInfo);
                //OutInfo.Diagnostics.push_back(Diagnostic(Lhs.PartByteOffsets[0],Lhs.Names[0].size(),
                //        "Error in assignment of non terminal \""+NonTermName+"\": "
                //        "Error in assignment to member \""+Component.AssignedMember.GetType<MemberReference>().Names[0]+"\": "
                //        +Error));
        }
    }
    //Parse def already verifies that all links between struct and non-terminal/terminal is true
    //here we only have to verify wheter or not the parse rules and structures abide by the semantics
    void MBCCDefinitions::p_UpdateReferencesAndVerify(LSPInfo& OutInfo)
    {
        if(NonTerminals.size() == 0)
        {
            //OutInfo.Diagnostics.push_back(Diagnostic(0,5,"Non-Ter"));
        }
        if(OutInfo.Diagnostics.size() == 0)
        {
            p_VerifyStructs(OutInfo);
            if(OutInfo.Diagnostics.size() == 0)
            {
                p_VerifyRules(OutInfo);
            }
        }
    }
    MBCCDefinitions MBCCDefinitions::ParseDefinitions(const char* Data,size_t DataSize,size_t InOffset,LSPInfo& OutInfo)
    {
        MBCCDefinitions ReturnValue; 
        size_t ParseOffset = InOffset; 
        MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        std::vector<std::pair<Identifier,Identifier>> UnresolvedDefs;
        std::unordered_map<std::string,std::vector<std::string>> UnresolvedLambdaLinks;
        int CurrentLamdaID = 1;
        while(ParseOffset < DataSize)
        {
            Identifier CurrentIdentifier = p_ParseIdentifier(Data,DataSize,ParseOffset,&ParseOffset);
            if(CurrentIdentifier.Value == "def")
            {
                OutInfo.SemanticsTokens.push_back(DefinitionsToken(CurrentIdentifier,DefinitionsTokenType::Keyword));
                auto NewDef = p_ParseDef(Data,DataSize,ParseOffset,&ParseOffset,OutInfo); 
                UnresolvedDefs.push_back(NewDef);
            }
            else if(CurrentIdentifier.Value == "term")
            {
                OutInfo.SemanticsTokens.push_back(DefinitionsToken(CurrentIdentifier,DefinitionsTokenType::Keyword));
                Terminal NewTerminal = p_ParseTerminal(Data,DataSize,ParseOffset,&ParseOffset,OutInfo);
                if(ReturnValue.NameToTerminal.find(NewTerminal.Name) != ReturnValue.NameToTerminal.end())
                {
                    OutInfo.Diagnostics.push_back(Diagnostic(CurrentIdentifier,"duplicate definition for terminal \""+NewTerminal.Name+"\""));
                }
                else if(ReturnValue.NameToNonTerminal.find(NewTerminal.Name) != ReturnValue.NameToNonTerminal.end())
                {
                    OutInfo.Diagnostics.push_back(Diagnostic(CurrentIdentifier,"attempting to define terminal with the same name as a nonterminal named \""+NewTerminal.Name+"\""));
                }
                else
                {
                    size_t TerminalIndex = ReturnValue.Terminals.size();
                    OutInfo.TerminalDefinitions[NewTerminal.Name] = CurrentIdentifier.ByteOffset;
                    ReturnValue.NameToTerminal[NewTerminal.Name] = TerminalIndex;
                    ReturnValue.Terminals.push_back(std::move(NewTerminal));
                }

            }
            else if(CurrentIdentifier.Value == "struct")
            {
                OutInfo.SemanticsTokens.push_back(DefinitionsToken(CurrentIdentifier,DefinitionsTokenType::Keyword));
                StructDefinition NewStruct = p_ParseStruct(Data,DataSize,ParseOffset,&ParseOffset,OutInfo);
                if(ReturnValue.NameToStruct.find(NewStruct.Name) != ReturnValue.NameToStruct.end())
                {
                    OutInfo.Diagnostics.push_back(Diagnostic(CurrentIdentifier,"duplicate definition for terminal \""+NewStruct.Name+"\""));
                }
                else
                {
                    size_t StructIndex = ReturnValue.Structs.size();
                    OutInfo.StructureDefinitions[NewStruct.Name] = CurrentIdentifier.ByteOffset;
                    ReturnValue.NameToStruct[NewStruct.Name] = StructIndex;
                    ReturnValue.Structs.push_back(std::move(NewStruct));
                }
            }
            else if(CurrentIdentifier.Value == "skip")
            {
                OutInfo.SemanticsTokens.push_back(DefinitionsToken(CurrentIdentifier,DefinitionsTokenType::Keyword));
                if(ReturnValue.SkipRegex != "")
                {
                    OutInfo.Diagnostics.push_back(Diagnostic(CurrentIdentifier,"there can only be one skip regex"));
                }
                MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
                if(ParseOffset  >= DataSize || Data[ParseOffset] != '"')
                {
                    throw MBCCParseError("Syntax error parsing MBCCDefinitions: no regex after skip declaration",ParseOffset);   
                }
                DefinitionsToken NewToken;
                NewToken.ByteOffset = ParseOffset;
                MBError ParseError = true;
                ReturnValue.SkipRegex = h_ParseRegex(Data,DataSize,ParseOffset,&ParseOffset,&ParseError);
                NewToken.Length = ParseOffset-NewToken.ByteOffset;
                NewToken.Type = DefinitionsTokenType::String;
                OutInfo.SemanticsTokens.push_back(NewToken);
                if(!ParseError)
                {
                    throw MBCCParseError("Syntax error parsing MBCC definitions: Error parsing skip statement: "+ParseError.ErrorMessage,ParseOffset);
                }
                MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
                if(ParseOffset >= DataSize || Data[ParseOffset] != ';')
                {
                    throw MBCCParseError("Syntax error parsing MBCC definitions: Skip statement needs delimiting ; to mark end",ParseOffset);
                }
                ParseOffset += 1;
            }
            else
            {
                MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
                if(ParseOffset >= DataSize || Data[ParseOffset] != '=')
                {
                    throw MBCCParseError("Syntactic error parsing MBCC definitions: rule needs delimiting = for name and content",ParseOffset);
                }
                ParseOffset +=1;
                MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);

                std::vector<Lambda> Lambdas;
                OutInfo.SemanticsTokens.push_back(DefinitionsToken(CurrentIdentifier,DefinitionsTokenType::NonTerminal));
                std::vector<ParseRule> NewRules = p_ParseParseRules(Data,DataSize,ParseOffset,&ParseOffset,';',CurrentLamdaID,Lambdas,OutInfo);


                NonTerminal NewTerminal;
                NewTerminal.Rules = std::move(NewRules);
                NewTerminal.Name = CurrentIdentifier.Value;
                auto NonTermIt = ReturnValue.NameToNonTerminal.find(NewTerminal.Name); 
                if(NonTermIt != ReturnValue.NameToNonTerminal.end())
                {
                    NonTerminal& AssociatedNonTerminal = ReturnValue.NonTerminals[NonTermIt->second];
                    AssociatedNonTerminal.Rules.insert(AssociatedNonTerminal.Rules.end(),std::make_move_iterator(NewTerminal.Rules.begin()),
                            std::make_move_iterator(NewTerminal.Rules.end()));
                }
                else
                {
                    if(ReturnValue.NameToTerminal.find(NewTerminal.Name) != ReturnValue.NameToTerminal.end())
                    {
                        OutInfo.Diagnostics.push_back(Diagnostic(CurrentIdentifier,"attempting to define non-terminal with the same name as a terminal named \""+NewTerminal.Name+"\""));
                    }
                    else
                    {
                        size_t CurrentIndex = ReturnValue.NonTerminals.size();
                        OutInfo.NonTerminalDefinitions[NewTerminal.Name] = CurrentIdentifier.ByteOffset;
                        ReturnValue.NameToNonTerminal[NewTerminal.Name] = CurrentIndex;
                        ReturnValue.NonTerminals.push_back(std::move(NewTerminal));
                    }
                }

                //lambdas are easier, they never collide with other lambdas,  and they never have the same name as another
                //terminal/terminals
                for(auto& NewLambda : Lambdas)
                {
                    NonTerminal NewNonTerminal;
                    NewNonTerminal.Name = NewLambda.Name;
                    NewNonTerminal.Rules = std::move(NewLambda.Rules);
                    auto& LambdaID = ReturnValue.NameToNonTerminal[NewNonTerminal.Name];
                    if(LambdaID == 0)
                    {
                        LambdaID = ReturnValue.NonTerminals.size();
                    }
                    Identifier TermName;
                    TermName.Value = NewNonTerminal.Name;
                    if(NewLambda.Type.Value != "")
                    {
                        UnresolvedDefs.push_back({TermName,NewLambda.Type});
                    }
                    else
                    {
                        UnresolvedLambdaLinks[CurrentIdentifier.Value].push_back(NewLambda.Name);
                        NewNonTerminal.IsInline = true;
                    }
                    ReturnValue.NonTerminals.push_back(std::move(NewNonTerminal));
                }
            }
            MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        }
        for(auto const& Def : UnresolvedDefs)
        {
            if(ReturnValue.NameToNonTerminal.find(Def.first.Value) == ReturnValue.NameToNonTerminal.end())
            {
                OutInfo.Diagnostics.push_back(Diagnostic(Def.first,"def referencing undefined rule \""+Def.first.Value+"\""));
            }
            else if(ReturnValue.NameToStruct.find(Def.second.Value) == ReturnValue.NameToStruct.end())
            {
                OutInfo.Diagnostics.push_back(Diagnostic(Def.second,"def referencing undefined struct \""+Def.second.Value+"\""));
            }
            else
            {
                auto LinkIt = UnresolvedLambdaLinks.find(Def.first.Value);
                auto StructValue = ReturnValue.NameToStruct[Def.second.Value];
                if(LinkIt != UnresolvedLambdaLinks.end())
                {
                    for(auto& LambdaName : LinkIt->second)
                    {
                        ReturnValue.NonTerminals[ReturnValue.NameToNonTerminal[LambdaName]].AssociatedStruct = StructValue;
                    }
                }
                ReturnValue.NonTerminals[ReturnValue.NameToNonTerminal[Def.first.Value]].AssociatedStruct = ReturnValue.NameToStruct[Def.second.Value];
            }
        }
        for(auto& NonTerm : ReturnValue.NonTerminals)
        {
            if(NonTerm.AssociatedStruct == -1)
            {
                auto StructIt = ReturnValue.NameToStruct.find(NonTerm.Name);
                if(StructIt != ReturnValue.NameToStruct.end())
                {
                    auto LinkIt = UnresolvedLambdaLinks.find(NonTerm.Name);
                    if(LinkIt != UnresolvedLambdaLinks.end())
                    {
                        for(auto& LambdaName : LinkIt->second)
                        {
                            ReturnValue.NonTerminals[ReturnValue.NameToNonTerminal[LambdaName]].AssociatedStruct = StructIt->second;
                        }
                    }
                    NonTerm.AssociatedStruct = StructIt->second;
                }
            }
        }
        if(ReturnValue.SkipRegex == "")
        {
            OutInfo.Diagnostics.push_back(Diagnostic(0,5,"Skip regex is mandatory in order to construct tokenizer"));
        }
        ReturnValue.p_UpdateReferencesAndVerify(OutInfo);
        std::sort(OutInfo.SemanticsTokens.begin(),OutInfo.SemanticsTokens.end());
        return(ReturnValue);
    }
    int h_OffsetToLine(const char* Data,size_t ParseOffset)
    {
        int ReturnValue = 1;    
        for(size_t i = 0; i < ParseOffset;i++)
        {
            if(Data[i] == '\n')
            {
                ReturnValue += 1;
            }   
        }
        return(ReturnValue);
    }
    MBCCDefinitions MBCCDefinitions::ParseDefinitions(const char* Data,size_t DataSize,size_t ParseOffset)
    {
        MBCCDefinitions ReturnValue; 
        LSPInfo OutInfo;
        ReturnValue = ParseDefinitions(Data,DataSize,ParseOffset,OutInfo); 
        if(OutInfo.Diagnostics.size() != 0)
        {
            throw std::runtime_error("Semantic error parsing definitions: "+OutInfo.Diagnostics[0].Message);
        }
        //catch(MBCCParseError const& Exception)
        //{
        //    MBLSP::Diagnostic NeWDiagnostic;
        //    NeWDiagnostic.message = "Error at line " +std::to_string(h_OffsetToLine(Data,Exception.ParseOffset))+" "+ Exception.ErrorMessage;
        //}
        //catch(std::exception const& e)
        //{
        //    MBLSP::Diagnostic NeWDiagnostic;
        //    //OutError = e.what(); 
        //}
        return(ReturnValue);
    }
    //END MBCCDefinitions
    
    //BEGIN GLA
    GLA::GLA(MBCCDefinitions const& Grammar,int k)
    {
        size_t TotalNodeSize = 2*Grammar.NonTerminals.size();
        m_NonTerminalCount = Grammar.NonTerminals.size();
        m_TerminalCount = Grammar.Terminals.size();
        for(auto const& NonTerminal : Grammar.NonTerminals)
        {
            for(auto const& Rule : NonTerminal.Rules)
            {
                TotalNodeSize += Rule.Components.size()+1;
            }   
        }
        m_Nodes = std::vector<GLANode>(TotalNodeSize,GLANode(k));
        //m_ProductionBegin = std::vector<NodeIndex>(Grammar.NonTerminals.size());
        int RuleOffset = Grammar.NonTerminals.size()*2;
        for(NonTerminalIndex i = 0; i < Grammar.NonTerminals.size();i++)
        {
            auto const& NonTerminal = Grammar.NonTerminals[i];
            for(auto const& Rule : NonTerminal.Rules)
            {
                m_Nodes[i].Edges.push_back(GLAEdge(-1,RuleOffset));
                for(auto const& Component : Rule.Components)
                {
                    //if(Component.ReferencedRule.Names[0] == "TOKEN")
                    //{
                    //    continue;
                    //}
                    if(Component.ReferencedRule.IsType<Literal>())
                    {
                        m_Nodes[RuleOffset].Edges.push_back(GLAEdge(-1,RuleOffset+1));
                    }
                    else if(Component.IsTerminal)
                    {
                        m_Nodes[RuleOffset].Edges.push_back(GLAEdge(Component.ComponentIndex,RuleOffset+1));
                        if (Component.Max == -1 || Component.Min == 0)
                        {
                            //SKIP
                            m_Nodes[RuleOffset].Edges.push_back(GLAEdge(-1, RuleOffset + 1));
                        }
                    }
                    else
                    {
                        //Position to start of non terminal
                        m_Nodes[RuleOffset].Edges.push_back(GLAEdge(-1,Component.ComponentIndex));
                        //Terminal to position in parsing state
                        m_Nodes[Grammar.NonTerminals.size()+Component.ComponentIndex].Edges.push_back(GLAEdge(-1,RuleOffset+1));

                        //TODO think about wheter this kind of cycle might affect the ability for the algorithm to do it's thing...
                        //TODO think about wheter or not duplicate edges should be handled in a better way
                        //TODO The article uses LOOk instead of FIRST and FOLLOW, so the case of when a Non terminal contains 
                        //fewer than K symbols in First might make it require a different step for the case of A*, it might
                        //be needed to be treated as a different terminal altogether
                        
                        //ADD FOLLOW from non terminal to current parse state, and add a skip to current parse state
                        if(Component.Min == 0)
                        {
                            //SKIP
                            m_Nodes[RuleOffset].Edges.push_back(GLAEdge(-1,RuleOffset+1));
                        }
                        if(Component.Max == -1)
                        {
                            //FOLLOW, terminal can follow itself
                            m_Nodes[Grammar.NonTerminals.size()+Component.ComponentIndex].Edges.push_back(GLAEdge(-1,Component.ComponentIndex));
                        }
                    }
                    RuleOffset++;
                }
                //add end?
                m_Nodes[RuleOffset].Edges.push_back(GLAEdge(Grammar.Terminals.size(),Grammar.NonTerminals.size()+i));
                RuleOffset++;
            }
        }
    }
    void h_Combine(std::vector<bool>& lhs,std::vector<bool>& rhs)
    {
        for(int i = 0; i < lhs.size();i++)
        {
            lhs[i] = lhs[i] || rhs[i];  
        } 
    }
    //NOTE exponential time implementation
    std::vector<bool> GLA::p_LOOK(GLANode& Node,int k) const
    {
        std::vector<bool> ReturnValue = std::vector<bool>(m_TerminalCount+1,false);
        if(k == -1)
        {
            return(ReturnValue);   
        }
        if(Node.Visiting[k])
        {
            return(ReturnValue);   
        }
        Node.Visiting[k] = true;
        for(auto const& Edge : Node.Edges)
        {
            if(Edge.ConnectionTerminal != -1)
            {
                if(k == 0)
                {
                    ReturnValue[Edge.ConnectionTerminal] = true;
                    //EOF marker, special in that it both counts as a terminal
                    //and should continue the search
                    if(Edge.ConnectionTerminal == m_TerminalCount)
                    {
                        std::vector<bool> SubValues = p_LOOK(m_Nodes[Edge.Connection],0);
                        h_Combine(ReturnValue,SubValues);
                    }
                }   
                else
                {
                    std::vector<bool> SubValues = p_LOOK(m_Nodes[Edge.Connection],k-1);
                    h_Combine(ReturnValue,SubValues);
                }
            }
            else
            {
                std::vector<bool> SubValues = p_LOOK(m_Nodes[Edge.Connection],k);   
                h_Combine(ReturnValue,SubValues);
            }
        }
        Node.Visiting[k] = false;
        return(ReturnValue);
    }
    MBMath::MBDynamicMatrix<bool> GLA::LOOK(NonTerminalIndex NonTerminal,int ProductionIndex,int k) const
    {
        MBMath::MBDynamicMatrix<bool> ReturnValue(m_TerminalCount+1,k);
        auto& Node = m_Nodes[m_Nodes[NonTerminal].Edges[ProductionIndex].Connection];
        for(int i = 0; i < k;i++)
        {
            MBMath::MBDynamicMatrix<bool> Visited(k,m_Nodes.size());
            std::vector<bool> CurrentLook = p_LOOK(Node,i);
            for(int j = 0; j < m_TerminalCount+1;j++)
            {
                ReturnValue(j,i) = CurrentLook[j];
            }
        }
        return(ReturnValue);
    }
    //MBMath::MBDynamicMatrix<bool> GLA::FIRST(NonTerminalIndex NonTerminal,int k)
    //{
    //    MBMath::MBDynamicMatrix<bool> ReturnValue;

    //    return(ReturnValue);
    //}
    //END GLA

    bool Tokenizer::IsEOF(Token const& TokenToExamine)
    {
        return(TokenToExamine.Type == m_TerminalRegexes.size()+1);
    }
    Token Tokenizer::p_ExtractToken() 
    {
        Token ReturnValue;
        ReturnValue.Type = m_TerminalRegexes.size()+1;
        if(m_ParseOffset == m_TextData.size())
        {
            return(ReturnValue);
        }
        std::smatch Match;
        std::string const& TextRef = m_TextData;
        if(std::regex_search(TextRef.begin()+m_ParseOffset,TextRef.end(),Match,m_Skip,std::regex_constants::match_continuous))
        {
            assert(Match.size() == 1);
            size_t SkipCount = Match[0].length();
            for(size_t i = m_ParseOffset; i < m_ParseOffset+SkipCount;i++)
            {
                if(m_TextData[i] == '\n')
                {
                    m_LineOffset += 1;
                    m_LineByteOffset = 0;
                }   
                else
                {
                    m_LineByteOffset += 1;   
                }
            }
            m_ParseOffset += SkipCount;
        }
        if(m_ParseOffset == m_TextData.size())
        {
            return(ReturnValue);
        }
        for(TerminalIndex i = 0; i < m_TerminalRegexes.size();i++)
        {
            if(std::regex_search(TextRef.begin()+m_ParseOffset,TextRef.end(),Match,m_TerminalRegexes[i].first,std::regex_constants::match_continuous))
            {
                //assert(Match.size() == 1);        
                if(!(m_TerminalRegexes[i].second < Match.size()))
                {
                    throw std::runtime_error("Regex at index "+std::to_string(i)+" have insufficient submatches");
                }
                ReturnValue.Value = Match[m_TerminalRegexes[i].second].str();
                ReturnValue.Type = i;
                ReturnValue.ByteOffset = m_ParseOffset;
                ReturnValue.Position = TokenPosition(m_LineOffset,m_LineByteOffset);
                size_t SkipCount = Match[0].length();
                for(size_t i = m_ParseOffset; i < m_ParseOffset+SkipCount;i++)
                {
                    if(m_TextData[i] == '\n')
                    {
                        m_LineOffset += 1;
                        m_LineByteOffset = 0;
                    }   
                    else
                    {
                        m_LineByteOffset += 1;   
                    }
                }
                m_ParseOffset += SkipCount;
                break;
            }
        }     
        if(ReturnValue.Type == m_TerminalRegexes.size()+1)
        {
            auto LineAndPosition = p_GetLineAndPosition(m_ParseOffset);
            throw std::runtime_error("Invalid character sequence: no terminal matching input at line "+std::to_string(LineAndPosition.first) +" and column " + std::to_string(LineAndPosition.second));
        }
        return(ReturnValue);
    }
    std::string Tokenizer::GetPositionString() const
    {
        std::string ReturnValue;       
        size_t ByteOffset = 0;
        if(m_StoredTokens.size() > 0)
        {
            ByteOffset = m_StoredTokens.front().ByteOffset;
        }
        else
        {
            ByteOffset = m_ParseOffset;   
        }
        auto LineAndPosition = p_GetLineAndPosition(m_ParseOffset);
        ReturnValue = "line "+std::to_string(LineAndPosition.first)+", col "+std::to_string(LineAndPosition.second);
        return(ReturnValue);
    }
    std::pair<int,int> Tokenizer::p_GetLineAndPosition(size_t TargetPosition) const
    {
        std::pair<int,int> ReturnValue = {1,1};
        size_t ParseOffset = 0;
        while(ParseOffset < TargetPosition)
        {
            size_t NextNewline = m_TextData.find('\n',ParseOffset);
            if(NextNewline == m_TextData.npos)
            {
                ReturnValue.second = TargetPosition-ParseOffset; 
                break;
            }
            else
            {
                ReturnValue.first += 1;
                ParseOffset = NextNewline+1;
            }
        }
        return ReturnValue;
    }
    Tokenizer::Tokenizer(std::string const& SkipRegex,std::initializer_list<std::string> TerminalRegexes)
    {
        m_Skip = std::regex(SkipRegex,std::regex_constants::ECMAScript|std::regex_constants::nosubs); 
        for(auto const& String : TerminalRegexes)
        {
            if(String.size() != 0 && String[0] == '$')
            {
                m_TerminalRegexes.push_back(std::pair<std::regex,int>(std::regex(String.substr(1),std::regex_constants::ECMAScript),1));
            }
            else
            {
                m_TerminalRegexes.push_back(std::pair<std::regex,int>(std::regex(String,std::regex_constants::ECMAScript|std::regex_constants::nosubs),0));
            }
        }
    }
    void Tokenizer::SetText(std::string NewText)
    {
        m_ParseOffset = 0;       
        m_LineByteOffset = 0;
        m_LineOffset = 0;
        m_TextData = std::move(NewText);
        m_StoredTokens.clear();
    }
    void Tokenizer::ConsumeToken()
    {
        if(m_StoredTokens.size() > 0)
        {
            m_StoredTokens.pop_front();
        }    
        else
        {
            p_ExtractToken();
        }
    }
    Token const& Tokenizer::Peek(int Depth)
    {
        while(m_StoredTokens.size() <= Depth)
        {
            m_StoredTokens.push_back(p_ExtractToken()); 
        }
        return(m_StoredTokens[Depth]);
    }
    //BEGIN LLParserGenerator
    std::vector<bool> CalculateENonTerminals(MBCCDefinitions const& Grammar)
    {
        std::vector<bool> ReturnValue = std::vector<bool>(Grammar.NonTerminals.size(),false);
        //Transitive closure algorithm, naive implementation
        while(true)
        {
            bool HasChanged = false;    
            for(int i = 0; i < Grammar.NonTerminals.size();i++)
            {
                if(ReturnValue[i])
                {
                    continue;   
                }
                auto const& NonTerminal = Grammar.NonTerminals[i];   
                for(auto const& Rule : NonTerminal.Rules)
                {
                    bool IsERule = true;
                    for(auto const& Component : Rule.Components)
                    {
                        if(Component.IsTerminal)
                        {
                            IsERule = false;
                            break;   
                        }
                        if(Component.Min != 0 && !ReturnValue[Component.ComponentIndex])
                        {
                            IsERule = false;
                            break;
                        }
                    }  
                    if(IsERule)
                    {
                        HasChanged = true;
                        ReturnValue[i] = true;
                        break;
                    }
                }
            }
            if(!HasChanged)
            {
                break;
            }
        }
        return ReturnValue;         
    }
    //Maybe kinda slow, should do a proper ordo analysis of the algorithm
    void p_VerifyNonTerminalLeftRecursive(NonTerminalIndex CurrentIndex,std::vector<bool>& VisitedTerminals,std::vector<bool> const& ERules,MBCCDefinitions const& Grammar)
    {
        if(VisitedTerminals[CurrentIndex] == true)
        {
            throw std::runtime_error("Error creating LL parser for grammar: NonTerminal \""+Grammar.NonTerminals[CurrentIndex].Name+ "\" is left recursive");    
        }
        VisitedTerminals[CurrentIndex] = true;
        for(auto const& Rule : Grammar.NonTerminals[CurrentIndex].Rules)
        {
            for(auto const& Component : Rule.Components)
            {
                if(Component.IsTerminal)
                {
                    break;
                }
                p_VerifyNonTerminalLeftRecursive(Component.ComponentIndex,VisitedTerminals,ERules,Grammar);
                if(!(Component.Min == 0 || ERules[Component.ComponentIndex]))
                {
                    break; 
                }
            }  
        }
        VisitedTerminals[CurrentIndex] = false;
    }
    void ParserCompilerHandler::VerifyNotLeftRecursive(MBCCDefinitions const& Grammar,std::vector<bool> const& ERules)
    {
        for(int i = 0; i <  Grammar.NonTerminals.size();i++)
        {
            std::vector<bool> VisitedTerminals = std::vector<bool>(Grammar.NonTerminals.size(),false);
            p_VerifyNonTerminalLeftRecursive(i,VisitedTerminals,ERules,Grammar);
        }
    }
    BoolTensor::BoolTensor(int i,int j,int k)
    {
        m_Data = std::vector<bool>(i * j * k,false);        
        m_J = j;
        m_K = k;
    }
    void BoolTensor::SetValue(int i,int j,int k)
    {
        m_Data[i*(m_J*m_K) + (j*m_K)+k] = true;
    }
    bool BoolTensor::GetValue(int i, int j, int k) const
    {
        return(m_Data[i*(m_J*m_K) + (j*m_K)+k]);
    }
    bool h_Disjunct(MBMath::MBDynamicMatrix<bool> const& lhs,MBMath::MBDynamicMatrix<bool> const& rhs)
    {
        bool ReturnValue = false;
        assert(lhs.NumberOfColumns() == rhs.NumberOfColumns() && lhs.NumberOfRows() == rhs.NumberOfRows());
        for(int k = 0; k < lhs.NumberOfColumns();k++)
        {
            bool IsDisjunct = true;
            //Hacky workaround because EOF is more common than it "should" be 
            //because the LOOK algorithm is a bit more pessimistic than true SLL',
            //if i understand it correctly
            for(int j = 0; j < lhs.NumberOfRows()-1;j++)
            {
                if(lhs(j,k) && rhs(j,k))
                {
                    IsDisjunct = false;    
                    break;
                }
            }
            if(IsDisjunct)
            {
                ReturnValue = true;   
                break;
            }
        }
        return(ReturnValue);
    }
    bool RulesAreDisjunct(std::vector<MBMath::MBDynamicMatrix<bool>> const& ProductionsToVerify)
    {
        bool ReturnValue = true;          
        for(int i = 0; i < ProductionsToVerify.size();i++)
        {
            for(int j = i+1; j < ProductionsToVerify.size();j++)
            {
                if(!h_Disjunct(ProductionsToVerify[i],ProductionsToVerify[j]))
                {
                    ReturnValue = false;   
                    break;
                }
            }    
        }
        return(ReturnValue);
    }
    std::string ParserCompilerHandler::Verify(MBCCDefinitions const& Grammar)
    {
        std::string ReturnValue;
        try
        {
            std::vector<bool> ERules = CalculateENonTerminals(Grammar); 
            VerifyNotLeftRecursive(Grammar,ERules);
        }
        catch(std::exception const& e)
        {
            ReturnValue = e.what();   
        }
        return(ReturnValue);
    }
    void ParserCompilerHandler::WriteParser(MBCCDefinitions const& Grammar,GrammarOptions const& Options,std::string const& LanguageName,std::string const& OutputBase)
    {
        std::vector<bool> ERules = CalculateENonTerminals(Grammar); 
        ParserCompilerHandler::VerifyNotLeftRecursive(Grammar,ERules);
        GLA GrammarGLA(Grammar,Options.k);
        std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> TotalProductions = ParserCompilerHandler::CalculateProductionsLinearApproxLOOK(Grammar,ERules,GrammarGLA,Options.k);
        NonTerminalIndex NonTermIndex = 0;
        for(auto const& Productions : TotalProductions)
        {
            if(!RulesAreDisjunct(Productions))
            {
                throw std::runtime_error("Error creating linear-approximate-LL("+std::to_string(Options.k)+") parser for grammar: Rule \""+Grammar.NonTerminals[NonTermIndex].Name+"\" is non deterministic");
            }
            NonTermIndex++;
        }
        auto CompIt = m_Compilers.find(LanguageName);
        if(CompIt == m_Compilers.end())
        {
            throw std::runtime_error("Error creating source code: no compiler for langauge \""+LanguageName+"\"");
        }
        CompIt->second->WriteParser(Grammar,TotalProductions,OutputBase);
        //CPPStreamIndenter HeaderIndent(&HeaderOut);
        //CPPStreamIndenter SourceIndent(&SourceOut);
        //p_WriteParser(Grammar,TotalProductions,HeaderName,HeaderIndent,SourceIndent);
    }
    ParserCompilerHandler::ParserCompilerHandler()
    {
        m_Compilers["cpp"] = std::make_unique<CPPParserGenerator>();
        m_Compilers["csharp"] = std::make_unique<CSharpParserGenerator>();
        m_Compilers["cpp-ir"] = std::make_unique<CppIRParser>();
    }
    std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> ParserCompilerHandler::CalculateProductionsLinearApproxLOOK(MBCCDefinitions const& Grammar,
    std::vector<bool> const& ERules,GLA const& GrammarGLA,int k)
    {
        std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> ReturnValue;
        NonTerminalIndex NonTermIndex = 0;
        for(auto const& NonTerminal : Grammar.NonTerminals)
        {
            std::vector<MBMath::MBDynamicMatrix<bool>> Productions = std::vector<MBMath::MBDynamicMatrix<bool>>(NonTerminal.Rules.size());
            for(int i = 0; i < NonTerminal.Rules.size();i++)
            {
                Productions[i] = GrammarGLA.LOOK(NonTermIndex,i,k);
            }
            ReturnValue.push_back(std::move(Productions));
            NonTermIndex++;
        }
        return(ReturnValue);
    }
    int h_CalculateDepth(StructIndex CurrentStructIndex,std::vector<bool>& Busy,std::vector<int>& OutDependancyDepth,std::vector<std::set<StructIndex>> const& StructureDependancies)
    {
        int ReturnValue = OutDependancyDepth[CurrentStructIndex];
        if(ReturnValue != -1)
        {
            return(ReturnValue);
        }
        if(Busy[CurrentStructIndex])
        {
            throw std::runtime_error("Error creating structure dependancies: Cyclic dependancies detected");   
        }
        Busy[CurrentStructIndex] = true;
        if(StructureDependancies[CurrentStructIndex].size() == 0)
        {
            ReturnValue = 0;   
        }
        else
        {
            int MaxDepDepth = -1;      
            for(auto const& Dependancy : StructureDependancies[CurrentStructIndex])
            {
                if(CurrentStructIndex == Dependancy)
                {
                   continue;    
                }
                int NewDepth = h_CalculateDepth(Dependancy,Busy,OutDependancyDepth,StructureDependancies);
                if(NewDepth > MaxDepDepth)
                {
                    MaxDepDepth = NewDepth;           
                }
            }
            ReturnValue = MaxDepDepth+1;
        }
        Busy[CurrentStructIndex] = false;
        OutDependancyDepth[CurrentStructIndex] = ReturnValue;
        return(ReturnValue);
    }
    std::vector<std::set<StructIndex>> h_CalculateStructDependancies(MBCCDefinitions const& Grammar,std::vector<std::set<StructIndex>>& OutChildrenInfo)
    {
        std::vector<std::set<StructIndex>> ReturnValue = std::vector<std::set<StructIndex>>(Grammar.Structs.size());
        for(StructIndex i = 0; i < Grammar.Structs.size();i++)
        {
            StructDefinition const& CurrentStruct = Grammar.Structs[i];
            if(CurrentStruct.ParentStruct != "")
            {
                StructIndex ParentIndex =  Grammar.NameToStruct.at(CurrentStruct.ParentStruct);
                ReturnValue[i].insert(ParentIndex);
                OutChildrenInfo[ParentIndex].insert(i);
            }
            for(auto const& Member : CurrentStruct.MemberVariables)
            {
                if(Member.IsType<StructMemberVariable_Struct>())
                {
                    ReturnValue[i].insert(Grammar.NameToStruct.at(Member.GetType<StructMemberVariable_Struct>().StructType));
                }
                else if(Member.IsType<StructMemberVariable_List>() && !TypeIsBuiltin(Member.GetType<StructMemberVariable_List>().ListType))
                {
                    ReturnValue[i].insert(Grammar.NameToStruct.at(Member.GetType<StructMemberVariable_List>().ListType));
                }
            }
        }
        return(ReturnValue);
    }
    DependancyInfo CalculateDependancyInfo(MBCCDefinitions const& Grammar)
    {
        DependancyInfo ReturnValue;           
        ReturnValue.StructureDependancyOrder = std::vector<StructIndex>(Grammar.Structs.size());
        ReturnValue.ChildrenMap = std::vector<std::set<StructIndex>>(Grammar.Structs.size());
        std::iota(ReturnValue.StructureDependancyOrder.begin(),ReturnValue.StructureDependancyOrder.end(),0);
        std::vector<std::set<StructIndex>> Dependancies = h_CalculateStructDependancies(Grammar,ReturnValue.ChildrenMap);
        std::vector<bool> Busy = std::vector<bool>(Grammar.Structs.size(),false);
        std::vector<int> DependancyDepth = std::vector<int>(Grammar.Structs.size(),-1);
        for(StructIndex i = 0; i < Grammar.Structs.size();i++)
        {
            h_CalculateDepth(i,Busy,DependancyDepth,Dependancies);
        }
        std::sort(ReturnValue.StructureDependancyOrder.begin(),ReturnValue.StructureDependancyOrder.end(),[&](StructIndex Lhs,StructIndex Rhs) -> bool
                {
                    return(DependancyDepth[Lhs] < DependancyDepth[Rhs]);
                });
        return(ReturnValue);
    }
    MBMath::MBDynamicMatrix<bool> CombineProductions(std::vector<MBMath::MBDynamicMatrix<bool>> const& MatrixesToCombine)
    {
        MBMath::MBDynamicMatrix<bool> ReturnValue = MBMath::MBDynamicMatrix<bool>(MatrixesToCombine[0]);
        for(size_t i = 1; i < MatrixesToCombine.size();i++)
        {
            MBMath::MBDynamicMatrix<bool> const& CurrentMatrix = MatrixesToCombine[i];
            for(int Row = 0; Row < CurrentMatrix.NumberOfRows();Row++)
            {
                for(int Column = 0; Column < CurrentMatrix.NumberOfRows();Column++)
                {
                    ReturnValue(Row,Column) = ReturnValue(Row,Column) || CurrentMatrix(Row,Column); 
                }    
            }
        }
        return(ReturnValue);
    }
    std::string h_GetTypeCppTypeString(StructMemberVariable const& Member)
    {
        std::string ReturnValue;   
        if(Member.IsType<StructMemberVariable_Struct>())
        {
            StructMemberVariable_Struct const& StructMember = Member.GetType<StructMemberVariable_Struct>();
            ReturnValue = StructMember.StructType;
        }
        else if(Member.IsType<StructMemberVariable_List>())
        {
            StructMemberVariable_List const& ListMember = Member.GetType<StructMemberVariable_List>();
            ReturnValue = "std::vector<"+ListMember.ListType+">";
        }
        else if(Member.IsType<StructMemberVariable_Bool>())
        {
            ReturnValue = "bool"; 
        }
        else if(Member.IsType<StructMemberVariable_tokenPosition>())
        {
            ReturnValue = "MBCC::TokenPosition"; 
        }
        else if(Member.IsType<StructMemberVariable_Int>())
        {
            ReturnValue = "int"; 
        }
        else if(Member.IsType<StructMemberVariable_Raw>())
        {
            StructMemberVariable_Raw const& RawMember = Member.GetType<StructMemberVariable_Raw>();
            ReturnValue = RawMember.RawMemberType;
        }
        else if(Member.IsType<StructMemberVariable_String>())
        {
            ReturnValue = "std::string"; 
        }
        else
        {
            assert(false && "h_GetTypeCppTypeString doesn't cover all cases, or Member stores an invalid type");   
        }
        return(ReturnValue);
    }
//END LLParserGenerator
    
    //BEGIN CPPStreamIndenter
    CPPStreamIndenter::CPPStreamIndenter(MBUtility::MBOctetOutputStream* StreamToConvert)
    {
        m_AssociatedStream = StreamToConvert;       
    }
    size_t CPPStreamIndenter::Write(const void* DataToWrite,size_t DataSize)
    {
        size_t ReturnValue = DataSize;       
        size_t ParseOffset = 0; 
        const char* CharData = (const char*)DataToWrite;
        while(ParseOffset < DataSize)
        {
            size_t NextLWing = std::find(CharData+ParseOffset,CharData+DataSize,'{')-CharData;
            size_t NextRWing = std::find(CharData+ParseOffset,CharData+DataSize,'}')-CharData;
            size_t NextNL = std::find(CharData+ParseOffset,CharData+DataSize,'\n')-CharData;
            if(NextNL != DataSize || NextRWing != DataSize || NextLWing != DataSize)
            {
                size_t Min = std::min(std::min(NextLWing,NextRWing),NextNL);
                m_AssociatedStream->Write(CharData+ParseOffset,Min+1-ParseOffset);
                ParseOffset = Min+1;
                if(Min == NextRWing)
                {
                    m_IndentLevel -=1;
                    if(m_IndentLevel < 0)
                    {
                        m_IndentLevel = 0;   
                    }
                }
                else if(Min == NextLWing)
                {
                    m_IndentLevel += 1;
                }
                else if(Min == NextNL)
                {
                    int IndentToWrite = m_IndentLevel;
                    if(ParseOffset < DataSize && CharData[ParseOffset] == '}' && IndentToWrite != 0)
                    {
                        IndentToWrite -=1;
                    }
                    char IndentData[4] = {' ',' ',' ',' '};
                    for(int i = 0; i < IndentToWrite;i++)
                    {
                        m_AssociatedStream->Write(IndentData,4);   
                    }
                }
            }
            else
            {
                m_AssociatedStream->Write(CharData+ParseOffset,DataSize-ParseOffset);    
                ParseOffset = DataSize;
            }
        }
        return(ReturnValue);
    }
    //END CPPStreamIndenter
}
