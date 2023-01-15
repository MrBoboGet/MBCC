#include "MBCC.h"
#include "MBUtility/MBErrorHandling.h"
#include <assert.h>
#include <iostream>
#include <regex>
#include <unordered_map>
#include <set>
#include <numeric>
#include <MBParsing/MBParsing.h>
namespace MBCC
{
    //BEGIN StructMemberVariable
    
    StructMemberVariable::StructMemberVariable(StructMemberVariable_List ListMemberVariable)
        : m_Content(std::move(ListMemberVariable))
    {
           
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
    std::string& StructMemberVariable::GetDefaultValue()
    {
        return(std::visit([&](MemberVariable& x) -> std::string& 
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
    std::string const& StructMemberVariable::GetDefaultValue() const
    {
        return(std::visit([&](MemberVariable const& x) -> std::string const& 
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
    class MBCCParseError : public std::exception
    {
    public:
        std::string ErrorMessage;
        size_t ParseOffset = 0;
        MBCCParseError(std::string Error,size_t NewParseOffset)
        {
            ErrorMessage = Error;
            ParseOffset = NewParseOffset;
        }
        const char* what() const override 
        {
            return(ErrorMessage.data());
        }
    };
    std::string MBCCDefinitions::p_ParseIdentifier(const char* Data,size_t DataSize,size_t InParseOffset,size_t* OutParseOffset)
    {
        std::string ReturnValue;
        size_t ParseOffset = InParseOffset;
        MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        size_t StringBegin = ParseOffset;
        while(ParseOffset < DataSize)
        {
            if(!((Data[ParseOffset] >= 65 && Data[ParseOffset] <= 90) || (Data[ParseOffset] >= 97 && Data[ParseOffset] <= 122)
                        || Data[ParseOffset] == '_' || (Data[ParseOffset] >= 48 && Data[ParseOffset] <= 57)))
            {
                break; 
            }   
            ParseOffset++;
        }
        ReturnValue = std::string(Data+StringBegin,Data+ParseOffset);
        if(ReturnValue == "")
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
    Terminal MBCCDefinitions::p_ParseTerminal(const char* Data,size_t DataSize,size_t InParseOffset,size_t* OutParseOffset)
    {
        Terminal ReturnValue;  
        size_t ParseOffset = InParseOffset;
        std::string TerminalName = p_ParseIdentifier(Data,DataSize,ParseOffset,&ParseOffset);
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
        MBError ParseStringResult = true;
        std::string RegexString = h_ParseRegex(Data,DataSize,ParseOffset,&ParseOffset,&ParseStringResult);
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
    StructMemberVariable MBCCDefinitions::p_ParseMemberVariable(const char* Data,size_t DataSize,size_t InParseOffset,size_t* OutParseOffset)
    {
        StructMemberVariable ReturnValue;
        size_t ParseOffset = InParseOffset;
        MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        if(ParseOffset >= DataSize)
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: end of file before member variable definition or end of struct",ParseOffset);   
        }
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
            std::string StructType = p_ParseIdentifier(Data,DataSize,ParseOffset,&ParseOffset); 
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
                List.ListType = p_ParseIdentifier(Data,DataSize,ParseOffset,&ParseOffset);
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
        std::string MemberVariableName = p_ParseIdentifier(Data,DataSize,ParseOffset,&ParseOffset);
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
            //A little bit of a hack, but doesnt require the parsing of any particular data
            size_t ValueEnd = std::find(Data+ParseOffset,Data+DataSize,';')-Data;
            if(ValueEnd >= DataSize)
            {
                throw MBCCParseError("Syntactic error parsing MBCC definitions: member variable needs delimiting ; for default value",ParseOffset);   
            }
            ReturnValue.GetDefaultValue() = std::string(Data+ParseOffset,Data+ValueEnd);
            ParseOffset = ValueEnd+1; 
        }
        else
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: member variable needs delimiting ;",ParseOffset);   
        }
        *OutParseOffset = ParseOffset;
        return(ReturnValue);
    }
    /*
struct Hej1 : Hej2
{
    List<Hej2> Hejs;
    {RawValue} RawTest;
}
    */
    StructDefinition MBCCDefinitions::p_ParseStruct(const char* Data,size_t DataSize,size_t InParseOffset,size_t* OutParseOffset)
    {
        StructDefinition ReturnValue;
        size_t ParseOffset = InParseOffset; 
        ReturnValue.Name = p_ParseIdentifier(Data,DataSize,ParseOffset,&ParseOffset);
        MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        if(ParseOffset < DataSize && Data[ParseOffset] == ':')
        {
            ParseOffset+=1;
            ReturnValue.ParentStruct = p_ParseIdentifier(Data,DataSize,ParseOffset,&ParseOffset);
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
            ReturnValue.MemberVariables.push_back(p_ParseMemberVariable(Data,DataSize,ParseOffset,&ParseOffset));
            MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        }
        if(!EndDelimiterFound)
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: struct needs delimiting } for end of member variables",ParseOffset);   
        }
        *OutParseOffset = ParseOffset;
        return(ReturnValue); 
    }
    std::pair<std::string,std::string> MBCCDefinitions::p_ParseDef(const char* Data,size_t DataSize,size_t InParseOffset,size_t* OutParseOffset)
    {
        std::pair<std::string,std::string> ReturnValue; 
        size_t ParseOffset = InParseOffset;
        std::string RuleName = p_ParseIdentifier(Data,DataSize,ParseOffset,&ParseOffset);
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
        std::string StructName = p_ParseIdentifier(Data,DataSize,ParseOffset,&ParseOffset);
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
        ReturnValue.first = RuleName;
        ReturnValue.second = StructName;
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
    std::vector<ParseRule> MBCCDefinitions::p_ParseParseRules(const char* Data,size_t DataSize,size_t InParseOffset,size_t* OutParseOffset)
    {
        std::vector<ParseRule> ReturnValue;
        size_t ParseOffset = InParseOffset;
        MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        if(ParseOffset >= DataSize || Data[ParseOffset] != '=')
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: rule needs delimiting = for name and content",ParseOffset);
        }
        ParseOffset +=1;
        MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        ParseRule CurrentRule;
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
                ParseOffset+=1;
                MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
                continue;
            }
            else if(Data[ParseOffset] == ';')
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
            std::string RuleName = p_ParseIdentifier(Data,DataSize,ParseOffset,&ParseOffset);    
            MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
            if(ParseOffset >= DataSize)
            {
                throw MBCCParseError("Syntactic error parsing MBCC definitions: missing ; in rule definition",ParseOffset);
            }
            if(Data[ParseOffset] == '=')
            {
                //member assignment    
                ParseOffset += 1;
                NewComponent.AssignedMember = RuleName;
                RuleName = p_ParseIdentifier(Data,DataSize,ParseOffset,&ParseOffset);
                MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
                if(ParseOffset >= DataSize)
                {
                    throw MBCCParseError("Syntactic error parsing MBCC definitions: missing ; in rule definition",ParseOffset);
                }
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
            NewComponent.ReferencedRule = RuleName;
            CurrentRule.Components.push_back(NewComponent);
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
    bool h_TypeIsBuiltin(std::string const& TypeToVerify)
    {
        return(TypeToVerify == "string" || TypeToVerify == "int" || TypeToVerify == "bool");
    }
    void MBCCDefinitions::p_VerifyStructs()
    {
        for(auto& Struct : Structs)
        {
            //Verify that the parent struct actually exists    
            if(Struct.ParentStruct != "" && NameToStruct.find(Struct.Name) == NameToStruct.end())
            {
                throw std::runtime_error("Semantic error parsing MBCC definitions: struct named \""+Struct.Name+"\" is the child of a non existing struct named \""+Struct.ParentStruct+"\"");
            } 
            for(auto& MemberVariable : Struct.MemberVariables)
            {
                if(MemberVariable.IsType<StructMemberVariable_List>())
                {
                    if(NameToStruct.find(MemberVariable.GetType<StructMemberVariable_List>().ListType) == NameToStruct.end())
                    {
                        throw std::runtime_error("Semantic error parsing MBCC definitions: List template value in struct \""+Struct.Name+ "\"references unknowns struct named \""+
                                MemberVariable.GetType<StructMemberVariable_List>().ListType+"\"");
                    }
                }
                else if(MemberVariable.IsType<StructMemberVariable_Struct>())
                {
                    StructMemberVariable_Struct& StructMember = MemberVariable.GetType<StructMemberVariable_Struct>();
                    if(h_TypeIsBuiltin(MemberVariable.GetType<StructMemberVariable_Struct>().StructType))
                    {
                        if(StructMember.StructType == "int")
                        {
                            StructMemberVariable_Int NewMember;
                            NewMember.Name = MemberVariable.GetName();
                            NewMember.DefaultValue = MemberVariable.GetDefaultValue();
                            if(NewMember.DefaultValue != "")
                            {
                                try
                                {
                                    NewMember.Value = std::stoi(NewMember.DefaultValue);
                                }
                                catch(std::exception const& e)
                                {
                                    throw std::runtime_error("Semantic error parsing MBCC definitions: Int member variable not a valid integer");
                                }
                            }
                            MemberVariable = StructMemberVariable(NewMember);
                        }
                        else if(StructMember.StructType == "string")
                        {
                            StructMemberVariable_String NewMember;
                            NewMember.Value = MemberVariable.GetDefaultValue();
                            NewMember.DefaultValue = MemberVariable.GetDefaultValue();
                            NewMember.Name = MemberVariable.GetName();
                            MemberVariable = StructMemberVariable(NewMember);
                               
                        }
                        else if(StructMember.StructType == "bool")
                        {
                            StructMemberVariable_Bool NewMember;   
                            NewMember.DefaultValue = MemberVariable.GetDefaultValue();
                            NewMember.Name = MemberVariable.GetName();
                            if(NewMember.DefaultValue != "")
                            {
                                size_t Offset = 0;
                                MBParsing::SkipWhitespace(NewMember.DefaultValue,0,&Offset);
                                if(NewMember.DefaultValue.size() -Offset == 4 && std::memcmp(NewMember.DefaultValue.data()+Offset,"true",4) == 0)
                                {
                                    NewMember.Value = true;   
                                }
                                else if(NewMember.DefaultValue.size() -Offset == 5 && std::memcmp(NewMember.DefaultValue.data()+Offset,"false",5) == 0)
                                {
                                    NewMember.Value = false;   
                                }
                                else
                                {
                                    throw std::runtime_error("Semantic error parsing MBCC definitions: invalid default value for bool type: "+NewMember.DefaultValue);   
                                }
                            }
                            MemberVariable = StructMemberVariable(NewMember);
                        }
                    }
                    else if(NameToStruct.find(MemberVariable.GetType<StructMemberVariable_Struct>().StructType) == NameToStruct.end())
                    {
                        throw std::runtime_error("Semantic error parsing MBCC definitions: member variable in struct \""+Struct.Name+"\" refernces unknowns struct named \""+
                                MemberVariable.GetType<StructMemberVariable_Struct>().StructType+"\"");
                    }
                }
            }
        }
        DepInfo = CalculateDependancyInfo(*this);
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
    //Assumption: Lhs and Rhs are valid struct indexes
    //Assumption: Only single Inheritance is supported
    bool MBCCDefinitions::p_IsAssignable(StructIndex Lhs,StructIndex Rhs)
    {
        bool ReturnValue = false;
        StructIndex CurrentIndex = Rhs;
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
    void MBCCDefinitions::p_VerifyRules()
    {
        for(auto& NonTerminal : NonTerminals)
        {
            StructDefinition* AssociatedStruct = nullptr;
            if(NonTerminal.AssociatedStruct != -1)
            {
                AssociatedStruct = &Structs[NonTerminal.AssociatedStruct];
            }
            for(auto& Rule : NonTerminal.Rules)
            {
                bool HasThisAssignment = false;
                bool HasRegularAssignment = false;
                for(auto& Component : Rule.Components)
                {
                    if(auto TermIt = NameToTerminal.find(Component.ReferencedRule); TermIt != NameToTerminal.end())
                    {
                        Component.IsTerminal = true;
                        Component.ComponentIndex = TermIt->second;
                    }   
                    else if(auto NonTermIt = NameToNonTerminal.find(Component.ReferencedRule); NonTermIt != NameToNonTerminal.end())
                    {
                        Component.IsTerminal = false;
                        Component.ComponentIndex = NonTermIt->second;              
                    }
                    else
                    {
                        throw std::runtime_error("Semantic error parsing MBCC definitions: "
                                "rule referencing unkown terminal/non-terminal named"
                                " \""+Component.ReferencedRule+"\"");
                    }
                    if(AssociatedStruct != nullptr)
                    {
                        if(Component.AssignedMember == "")
                        {
                            continue;   
                        }
                        if(Component.AssignedMember == "this")
                        {
                            HasThisAssignment = true;
                            if(Component.IsTerminal)
                            {
                                throw std::runtime_error("Semantic error parsing MBCC definitions: "
                                     "Error in member assignment for non-terminal \""+NonTerminal.Name+"\": "
                                     "Cannot assign terminal to this");
                            }
                            if(Component.Min != 1 || Component.Max != 1)
                            {
                                throw std::runtime_error("Semantic error parsing MBCC definitions: "
                                        "Error in member assignment for non-terminal \""+NonTerminal.Name+"\": "
                                        "Can only assign to this with terminal of exactly size 1");
                            }
                            //if(NonTerminal.AssociatedStruct != NonTerminals[Component.ComponentIndex].AssociatedStruct)
                            if(!p_IsAssignable(NonTerminal.AssociatedStruct,NonTerminals[Component.ComponentIndex].AssociatedStruct))
                            {
                                throw std::runtime_error("Semantic error parsing MBCC definitions: "
                                        "error in member assignment for non-terminal \""+NonTerminal.Name+"\": "
                                        "error assigning non-terminal \""+Component.ReferencedRule+"\" to this: "+
                                        "non-terminal has type \""+Structs[NonTerminal.AssociatedStruct].Name+"\" but"
                                        " assigned non-terminal is of type \""+Structs[NonTerminals[Component.ComponentIndex].AssociatedStruct].Name+"\"");

                            }
                            continue;
                        }
                        HasRegularAssignment = true;
                        //if(!AssociatedStruct->HasMember(Component.AssignedMember))
                        if(!HasMember(*AssociatedStruct,Component.AssignedMember))
                        {
                            throw std::runtime_error("Semantic error parsing MBCC definitions: "
                                    "rule \""+NonTerminal.Name+"\" referencing non existing member \""+Component.AssignedMember+"\" "
                                    "of struct \""+AssociatedStruct->Name+"\"");
                        }
                        //Check type of assigned member
                        StructMemberVariable const& AssociatedMember = GetMember(*AssociatedStruct,Component.AssignedMember);
                        if(AssociatedMember.IsType<StructMemberVariable_Struct>())
                        {
                            StructMemberVariable_Struct const& StructMember = AssociatedMember.GetType<StructMemberVariable_Struct>();  
                            if(Component.IsTerminal)
                            {
                                throw std::runtime_error("Semantic error parsing MBCC definitions: "
                                        "error in member assignment for non-terminal \""+NonTerminal.Name+"\": "
                                        "error assigning terminal \""+Component.ReferencedRule+"\" to member \""+StructMember.Name+
                                        "\": can only assign non-terminals to non builtin types");
                            }
                            //Struct have already been verified
                            //structmember uses 
                            //if(NameToStruct[StructMember.StructType] != NonTerminals[Component.ComponentIndex].AssociatedStruct)
                            if(!p_IsAssignable(NameToStruct[StructMember.StructType],NonTerminals[Component.ComponentIndex].AssociatedStruct))
                            {
                                throw std::runtime_error("Semantic error parsing MBCC definitions: "
                                        "error in member assignment for non-terminal \""+NonTerminal.Name+"\": "
                                        "error assigning non-terminal \""+Component.ReferencedRule+"\" to member \""+AssociatedMember.GetName()+"\": "+
                                        "member is of type "+Structs[NameToStruct[StructMember.StructType]].Name +" "
                                        "and non-terminal is of type "+Structs[NonTerminals[Component.ComponentIndex].AssociatedStruct].Name);
                            }
                        }
                        else if(AssociatedMember.IsType<StructMemberVariable_List>())
                        {
                            StructMemberVariable_List const& ListMember = AssociatedMember.GetType<StructMemberVariable_List>();  
                            if(Component.IsTerminal)
                            {
                                throw std::runtime_error("Semantic error parsing MBCC definitions: "
                                        "error in member assignment for non-terminal \""+NonTerminal.Name+"\": "
                                        "error assigning terminal \""+Component.ReferencedRule+"\" to member \""+ListMember.Name+
                                        "\": can only assign non-terminals to non builtin types");
                            }
                            //Struct have already been verified
                            //structmember uses 
                            //if(NameToStruct[ListMember.ListType] != NonTerminals[Component.ComponentIndex].AssociatedStruct)
                            if(!p_IsAssignable(NameToStruct[ListMember.ListType],NonTerminals[Component.ComponentIndex].AssociatedStruct))
                            {
                                throw std::runtime_error("Semantic error parsing MBCC definitions: "
                                        "error in member assignment for non-terminal \""+NonTerminal.Name+"\": "
                                        "error assigning non-terminal \""+Component.ReferencedRule+"\" to member \""+AssociatedMember.GetName()+"\": "+
                                        " member is of type "+Structs[NameToStruct[ListMember.ListType]].Name +" "
                                        "and non-terminal is of type "+Structs[NonTerminals[Component.ComponentIndex].AssociatedStruct].Name);
                            }
                        }
                        else
                        {
                            if(Component.IsTerminal == false)
                            {
                                throw std::runtime_error("Semantic error parsing MBCC definitions: "
                                        "error with member assignment in non-terminal \""+NonTerminal.Name+"\": "
                                        "only a terminal can be assigned to builtin scalar types");
                            }     
                        }
                    }
                    else
                    {
                        if(Component.AssignedMember != "")
                        {
                            throw std::runtime_error("Semantic error parsing MBCC definitions: "
                                    "non-terminal \""+NonTerminal.Name+"\" assigning to member \""+Component.AssignedMember+"\" "
                                    "but doesn't have any linked struct");
                        }
                    }
                } 
                if(HasRegularAssignment && HasThisAssignment)
                {
                    Rule.NeedsAssignmentOrder = true;   
                }
            }
        } 
    }
    //Parse def already verifies that all links between struct and non-terminal/terminal is true
    //here we only have to verify wheter or not the parse rules and structures abide by the semantics
    void MBCCDefinitions::p_UpdateReferencesAndVerify()
    {
        if(NonTerminals.size() == 0)
        {
            throw std::runtime_error("Semantic error parsing MBCC definitions: cannot construct a parser without any nonterminals");
        }
        p_VerifyStructs();
        p_VerifyRules();
    }
    MBCCDefinitions MBCCDefinitions::ParseDefinitions(const char* Data,size_t DataSize,size_t InOffset)
    {
        MBCCDefinitions ReturnValue; 
        size_t ParseOffset = InOffset; 
        MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        std::vector<std::pair<std::string,std::string>> UnresolvedDefs;
        while(ParseOffset < DataSize)
        {
            std::string CurrentIdentifier = p_ParseIdentifier(Data,DataSize,ParseOffset,&ParseOffset);
            if(CurrentIdentifier == "def")
            {
                auto NewDef = p_ParseDef(Data,DataSize,ParseOffset,&ParseOffset); 
                UnresolvedDefs.push_back(NewDef);
            }
            else if(CurrentIdentifier == "term")
            {
                Terminal NewTerminal = p_ParseTerminal(Data,DataSize,ParseOffset,&ParseOffset);
                if(ReturnValue.NameToTerminal.find(NewTerminal.Name) != ReturnValue.NameToTerminal.end())
                {
                    throw std::runtime_error("Semantic error parsing MBCC definitions: duplicate definition for terminal \""+NewTerminal.Name+"\"");
                }
                if(ReturnValue.NameToNonTerminal.find(NewTerminal.Name) != ReturnValue.NameToNonTerminal.end())
                {
                    throw std::runtime_error("Semantic error parsing MBCC definitions: attempting to define terminal with the same name as a nonterminal named \""+NewTerminal.Name+"\"");
                }
                size_t TerminalIndex = ReturnValue.Terminals.size();
                ReturnValue.NameToTerminal[NewTerminal.Name] = TerminalIndex;
                ReturnValue.Terminals.push_back(std::move(NewTerminal));

            }
            else if(CurrentIdentifier == "struct")
            {
                StructDefinition NewStruct = p_ParseStruct(Data,DataSize,ParseOffset,&ParseOffset);
                if(ReturnValue.NameToStruct.find(NewStruct.Name) != ReturnValue.NameToStruct.end())
                {
                    throw std::runtime_error("Semantic error parsing MBCC definitions: duplicate definition for struct \""+NewStruct.Name+"\"");
                }
                size_t StructIndex = ReturnValue.Structs.size();
                ReturnValue.NameToStruct[NewStruct.Name] = StructIndex;
                ReturnValue.Structs.push_back(std::move(NewStruct));
            }
            else if(CurrentIdentifier == "skip")
            {
                if(ReturnValue.SkipRegex != "")
                {
                    throw std::runtime_error("Semantic error parsing MBCC definitions: there can only be one skip regex");   
                }
                MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
                MBError ParseError = true;
                ReturnValue.SkipRegex = h_ParseRegex(Data,DataSize,ParseOffset,&ParseOffset,&ParseError);
                if(!ParseError)
                {
                    throw std::runtime_error("Syntax error parsing MBCC definitions: Error parsing skip statement: "+ParseError.ErrorMessage);
                }
                MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
                if(ParseOffset >= DataSize || Data[ParseOffset] != ';')
                {
                    throw std::runtime_error("Syntax error parsing MBCC definitions: Skip statement needs delimiting ; to mark end");
                }
                ParseOffset += 1;
            }
            else
            {
                std::vector<ParseRule> NewRules = p_ParseParseRules(Data,DataSize,ParseOffset,&ParseOffset);
                NonTerminal NewTerminal;
                NewTerminal.Rules = std::move(NewRules);
                NewTerminal.Name = CurrentIdentifier;
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
                        throw std::runtime_error("Semantic error parsing MBCC definitions: attempting to define non-terminal with the same name as a terminal named \""+NewTerminal.Name+"\"");
                    }
                    size_t CurrentIndex = ReturnValue.NonTerminals.size();
                    ReturnValue.NameToNonTerminal[NewTerminal.Name] = CurrentIndex;
                    ReturnValue.NonTerminals.push_back(std::move(NewTerminal));
                }
            }
            MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        }
        for(auto const& Def : UnresolvedDefs)
        {
            if(ReturnValue.NameToNonTerminal.find(Def.first) == ReturnValue.NameToNonTerminal.end())
            {
                throw std::runtime_error("Semantic error parsing MBCC definitions: def referencing undefined rule \""+Def.first+"\"");    
            }
            if(ReturnValue.NameToStruct.find(Def.second) == ReturnValue.NameToStruct.end())
            {
                throw std::runtime_error("Semantic error parsing MBCC definitions: def referencing undefined struct \""+Def.second+"\"");    
            }
            ReturnValue.NonTerminals[ReturnValue.NameToNonTerminal[Def.first]].AssociatedStruct = ReturnValue.NameToStruct[Def.second];
        }
        if(ReturnValue.SkipRegex == "")
        {
            throw std::runtime_error("Semantic error parsing MBCC definitions: Skip regex is mandatory in order to construct tokenizer");   
        }
        ReturnValue.p_UpdateReferencesAndVerify();
        return(ReturnValue);
    }
    int h_OffsetToLine(const char* Data,size_t ParseOffset)
    {
        int ReturnValue = 0;    
        for(size_t i = 0; i < ParseOffset;i++)
        {
            if(Data[i] == '\n')
            {
                ReturnValue += 1;
            }   
        }
        return(ReturnValue);
    }
    MBCCDefinitions MBCCDefinitions::ParseDefinitions(const char* Data,size_t DataSize,size_t ParseOffset,std::string& OutError)
    {
        MBCCDefinitions ReturnValue; 
        OutError = "";
        try
        {
            ReturnValue = ParseDefinitions(Data,DataSize,ParseOffset); 
        }
        catch(MBCCParseError const& Exception)
        {
            OutError ="Error at line " +std::to_string(h_OffsetToLine(Data,Exception.ParseOffset))+" "+ Exception.ErrorMessage;
        }
        catch(std::exception const& e)
        {
            OutError = e.what(); 
        }
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
                    if(Component.IsTerminal)
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
                        if(Component.Max == -1 || Component.Min == 0)
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
            m_ParseOffset += Match[0].length();    
        }
        if(m_ParseOffset == m_TextData.size())
        {
            return(ReturnValue);
        }
        for(TerminalIndex i = 0; i < m_TerminalRegexes.size();i++)
        {
            if(std::regex_search(TextRef.begin()+m_ParseOffset,TextRef.end(),Match,m_TerminalRegexes[i],std::regex_constants::match_continuous))
            {
                assert(Match.size() == 1);        
                ReturnValue.Value = Match[0].str();
                ReturnValue.Type = i;
                m_ParseOffset += ReturnValue.Value.size();
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
    std::pair<int,int> Tokenizer::p_GetLineAndPosition(size_t TargetPosition)
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
            m_TerminalRegexes.emplace_back(String,std::regex_constants::ECMAScript|std::regex_constants::nosubs);
        }
    }
    void Tokenizer::SetText(std::string NewText)
    {
        m_ParseOffset = 0;       
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
    std::vector<bool> LLParserGenerator::p_RetrieveENonTerminals(MBCCDefinitions const& Grammar)
    {
        return(CalculateENonTerminals(Grammar));
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
    void LLParserGenerator::VerifyNotLeftRecursive(MBCCDefinitions const& Grammar,std::vector<bool> const& ERules)
    {
        p_VerifyNotLeftRecursive(Grammar,ERules);
    }
    void LLParserGenerator::p_VerifyNotLeftRecursive(MBCCDefinitions const& Grammar,std::vector<bool> const& ERules)
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
            for(int j = 0; j < lhs.NumberOfRows();j++)
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
    bool h_RulesAreDisjunct(std::vector<MBMath::MBDynamicMatrix<bool>> const& ProductionsToVerify)
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
    void LLParserGenerator::p_WriteDefinitions(MBCCDefinitions const& Grammar,std::vector<TerminalStringMap> const& ParseTable,MBUtility::MBOctetOutputStream& HeaderOut,MBUtility::MBOctetOutputStream& SourceOut, int k)
    {
    }
    std::string LLParserGenerator::Verify(MBCCDefinitions const& Grammar)
    {
        std::string ReturnValue;
        try
        {
            std::vector<bool> ERules = p_RetrieveENonTerminals(Grammar); 
            p_VerifyNotLeftRecursive(Grammar,ERules);
        }
        catch(std::exception const& e)
        {
            ReturnValue = e.what();   
        }
        return(ReturnValue);
    }
    std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> LLParserGenerator::CalculateProductionsLinearApproxLOOK(MBCCDefinitions const& Grammar,
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
            if(!h_RulesAreDisjunct(Productions))
            {
                throw std::runtime_error("Error creating linear-approximate-LL("+std::to_string(k)+") parser for grammar: Rule \""+NonTerminal.Name+"\" is non deterministic");
            }
            ReturnValue.push_back(std::move(Productions));
            NonTermIndex++;
        }
        return(ReturnValue);
    }
    void LLParserGenerator::WriteLLParser(MBCCDefinitions const& Grammar,std::string const& HeaderName,MBUtility::MBOctetOutputStream& HeaderOut,MBUtility::MBOctetOutputStream& SourceOut,int k)
    {
        std::vector<bool> ERules = p_RetrieveENonTerminals(Grammar); 
        p_VerifyNotLeftRecursive(Grammar,ERules);
        GLA GrammarGLA(Grammar,k);
        std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> TotalProductions = CalculateProductionsLinearApproxLOOK(Grammar,ERules,GrammarGLA,k);
        CPPStreamIndenter HeaderIndent(&HeaderOut);
        CPPStreamIndenter SourceIndent(&SourceOut);
        p_WriteParser(Grammar,TotalProductions,HeaderName,HeaderIndent,SourceIndent);
    } 
    void LLParserGenerator::p_WriteParser(MBCCDefinitions const& Grammar,std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> const& ProductionsLOOk,
            std::string const& HeaderName,
        MBUtility::MBOctetOutputStream& HeaderOut,MBUtility::MBOctetOutputStream& SourceOut)
    {
        p_WriteSource(Grammar,ProductionsLOOk,HeaderName,SourceOut); 
        p_WriteHeader(Grammar,HeaderOut);
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
                else if(Member.IsType<StructMemberVariable_List>())
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
    void h_WriteStructures(MBCCDefinitions const& Grammar,DependancyInfo const& DepInfo,MBUtility::MBOctetOutputStream& HeaderOut)
    {
        std::vector<StructIndex> AbstractStructs;
        for(StructIndex CurrentIndex : DepInfo.StructureDependancyOrder)
        {
            StructDefinition const& CurrentStruct = Grammar.Structs[CurrentIndex];
            std::string StructName = CurrentStruct.Name;
            if(DepInfo.ChildrenMap[CurrentIndex].size() > 0)
            {
                StructName += "_Base";
                AbstractStructs.push_back(CurrentIndex);
            }
            HeaderOut << "class "<<StructName;
            if(CurrentStruct.ParentStruct != "")
            {
                HeaderOut<<" : public "<<CurrentStruct.ParentStruct;
                if(DepInfo.ChildrenMap[Grammar.NameToStruct.at(CurrentStruct.ParentStruct)].size() > 0)
                {
                    HeaderOut<<"_Base";   
                }
            }
            HeaderOut<<"\n{\n";
            HeaderOut<<"public:\n";
            for(auto const& Member : CurrentStruct.MemberVariables)
            {
                if(Member.IsType<StructMemberVariable_Int>())
                {
                    HeaderOut << "int ";     
                }
                else if(Member.IsType<StructMemberVariable_String>())
                {
                    HeaderOut << "std::string ";   
                }
                else if(Member.IsType<StructMemberVariable_Struct>())
                {
                    HeaderOut << Member.GetType<StructMemberVariable_Struct>().StructType<<" ";
                }
                else if(Member.IsType<StructMemberVariable_Bool>())
                {
                    HeaderOut << "bool ";
                }
                else if(Member.IsType<StructMemberVariable_List>())
                {
                    HeaderOut <<"std::vector<"<<Member.GetType<StructMemberVariable_List>().ListType<<"> ";
                }
                else if(Member.IsType<StructMemberVariable_Raw>())
                {
                    HeaderOut << Member.GetType<StructMemberVariable_Raw>().RawMemberType<<" ";
                }
                HeaderOut<<Member.GetName();
                if(Member.GetDefaultValue() != "")
                {
                    HeaderOut<<" = "<<Member.GetDefaultValue();   
                }
                HeaderOut <<";\n";

            }         
            HeaderOut<<"\n};\n";
            //If abstract class, write the containe class
            if(DepInfo.ChildrenMap[CurrentIndex].size() > 0)
            {
                std::string ConcreteStructName = Grammar.Structs[CurrentIndex].Name;
                HeaderOut << "class "<<ConcreteStructName;
                HeaderOut<<"\n{\n";
                HeaderOut<<"private:\n std::unique_ptr<"<<StructName<<"> m_Data;\nsize_t m_TypeID = 0;\n";
                HeaderOut<<"template<typename T> static size_t p_GetTypeID(){return size_t(&p_GetTypeID<T>);}\n";
                HeaderOut<<"public:\n";
                HeaderOut<<"template<typename T> void Accept(T& Visitor);\n";
                HeaderOut<<"template<typename T> void Accept(T& Visitor) const;\n";
                HeaderOut<<"template<typename T> "<< ConcreteStructName<< "(T ObjectToStore)\n{\nm_Data = std::unique_ptr<"<<StructName<<">(new T(std::move(ObjectToStore)));\nm_TypeID = p_GetTypeID<T>();\n}\n";
                HeaderOut<<ConcreteStructName << "() = default;\n";
                HeaderOut<<ConcreteStructName << "("<<ConcreteStructName<<"&&) = default;\n";
                HeaderOut<<"template<typename T> bool IsType() const\n{\nreturn m_TypeID == p_GetTypeID<T>();\n}\n";
                HeaderOut<<"bool IsEmpty() const\n{\nreturn m_Data == nullptr;\n}\n";
                HeaderOut<<"void operator=("<<ConcreteStructName<<"&& StructToMove)\n{\nstd::swap(m_TypeID,StructToMove.m_TypeID);\nstd::swap(m_Data,StructToMove.m_Data);\n}\n";
                HeaderOut<<"template<typename T> T const& GetType() const\n{\nif(!IsType<T>() || m_Data == nullptr)\n{\nthrow std::runtime_error(\"Invalid type access for abstract AST class\");\n}return static_cast<T const&>(*m_Data);\n}\n";
                HeaderOut<<"template<typename T> T& GetType()\n{\nif(!IsType<T>() || m_Data == nullptr)\n{\nthrow std::runtime_error(\"Invalid type access for abstract AST class\");\n}return static_cast<T&>(*m_Data);\n}\n";
                HeaderOut<<StructName<<"& GetBase()\n{\nif(m_Data == nullptr)\n{\nthrow std::runtime_error(\"Invalid type access for abstract AST class: data is null\");\n}return static_cast<"<< StructName<<"&>(*m_Data);\n}\n";
                HeaderOut<<StructName<<" const& GetBase() const\n{\nif(m_Data == nullptr)\n{\nthrow std::runtime_error(\"Invalid type access for abstract AST class: data is null\");\n}return static_cast<"<< StructName<<" const&>(*m_Data);\n}\n";
                HeaderOut<<"\n};\n";
            }
        }
        for(auto const& AbstractStructIndex : AbstractStructs)
        { 
            HeaderOut<<"template<typename T> void "<< Grammar.Structs[AbstractStructIndex].Name<<"::"<< "Accept(T& Visitor)"; 
            HeaderOut << "\n{\n";
            for(StructIndex ChildStruct : DepInfo.ChildrenMap[AbstractStructIndex])
            {
                HeaderOut<< "if(p_GetTypeID<"+Grammar.Structs[ChildStruct].Name+">() == m_TypeID)\n{\nVisitor(static_cast<"+Grammar.Structs[ChildStruct].Name+"&>(*m_Data));\n}\n";
                HeaderOut<< "else ";
            }
            HeaderOut<< "\n{\nthrow std::runtime_error(\"Invalid object stored in AST abstract class\");\n}\n";
            HeaderOut<< "\n}\n";


            HeaderOut<<"template<typename T> void "<< Grammar.Structs[AbstractStructIndex].Name<<"::"<< "Accept(T& Visitor) const"; 
            HeaderOut << "\n{\n";
            for(StructIndex ChildStruct : DepInfo.ChildrenMap[AbstractStructIndex])
            {
                HeaderOut<< "if(p_GetTypeID<"+Grammar.Structs[ChildStruct].Name+">() == m_TypeID)\n{\nVisitor(static_cast<"+Grammar.Structs[ChildStruct].Name+" const&>(*m_Data));\n}\n";
                HeaderOut<< "else ";
            }
            HeaderOut<< "\n{\nthrow std::runtime_error(\"Invalid object stored in AST abstract class\");\n}\n";
            HeaderOut<< "\n}\n";
        } 
        //Write traverser
        //HeaderOut<<"template<typename T> class Traverser\n{\nprotected:\nT* m_Traveler = nullptr;";
        //for(StructIndex CurrentStructIndex = 0; CurrentStructIndex < Grammar.Structs.size();CurrentStructIndex++)
        //{
        //    auto const& CurrentStruct = Grammar.Structs[CurrentStructIndex];
        //    HeaderOut<<"void operator("+CurrentStruct.Name+" const&)\n{\n";
        //}
        //HeaderOut<<"public:\n";
        //HeaderOut<<"template<typename A> void Traverse(T& Traveler,A& AST)\n{\nm_Traveler = &Traveler;\n(*this)(AST);\n}\n";

    }
    std::string h_CppLiteralEscapeString(std::string const& StringToEscape)
    {
        std::string ReturnValue;
        if(StringToEscape.empty())
        {
            return(ReturnValue);   
        }
        size_t ParseOffset = 0;
        while(ParseOffset < StringToEscape.size())
        {
            size_t NextSlash = StringToEscape.find('\\', ParseOffset);
            size_t NextQuote = StringToEscape.find('"',ParseOffset);
            size_t NextEscape = std::min(NextSlash,NextQuote);
            if(NextEscape == StringToEscape.npos)
            {
                ReturnValue.insert(ReturnValue.end(),StringToEscape.data()+ParseOffset,StringToEscape.data()+StringToEscape.size());
                break;
            }
            else
            {
                ReturnValue.insert(ReturnValue.end(),StringToEscape.data()+ParseOffset,StringToEscape.data()+NextEscape);
                ReturnValue += '\\';
                ReturnValue += StringToEscape[NextEscape];
                ParseOffset = NextEscape+1;
            }
        }
        return(ReturnValue);
    }
    void LLParserGenerator::p_WriteHeader(MBCCDefinitions const& Grammar, MBUtility::MBOctetOutputStream& HeaderOut)
    {

        HeaderOut << "#pragma once\n";
        HeaderOut << "#include <MBCC/MBCC.h>\n"; 
        HeaderOut << "#include <string>\n";
        HeaderOut << "#include <vector>\n";
        HeaderOut << "#include <stdexcept>\n";
        HeaderOut << "#include <variant>\n";
        HeaderOut << "#include <memory>\n";
        DependancyInfo DepInfo = Grammar.DepInfo;
        h_WriteStructures(Grammar,DepInfo,HeaderOut);
        p_WriteFunctionHeaders(Grammar,HeaderOut);
        HeaderOut<<"inline MBCC::Tokenizer GetTokenizer()\n{\nMBCC::Tokenizer ReturnValue(\""+h_CppLiteralEscapeString(Grammar.SkipRegex)+"\",{"; 
        for(auto const& Terminal : Grammar.Terminals)
        {
            HeaderOut<<"\""<<h_CppLiteralEscapeString(Terminal.RegexDefinition)<<"\",";   
        }
        HeaderOut <<"});\nreturn(ReturnValue);\n}";
    }
    void LLParserGenerator::p_WriteFunctionHeaders(MBCCDefinitions const& Grammar,MBUtility::MBOctetOutputStream& HeaderOut)
    {
        std::string ReturnValueType = "void";
        for(NonTerminalIndex NonTermIndex = 0; NonTermIndex < Grammar.NonTerminals.size();NonTermIndex++)
        {
            StructDefinition const* AssoicatedStruct = nullptr;
            NonTerminal const& AssociatedNonTerminal = Grammar.NonTerminals[NonTermIndex];
            if(AssociatedNonTerminal.AssociatedStruct != -1)
            {
                AssoicatedStruct = &Grammar.Structs[AssociatedNonTerminal.AssociatedStruct];    
            }
            if(AssoicatedStruct != nullptr)
            {
                ReturnValueType = AssoicatedStruct->Name;
            }
            MBUtility::WriteData(HeaderOut,ReturnValueType+" Parse"+AssociatedNonTerminal.Name+"(MBCC::Tokenizer& Tokenizer);\n");
            for(int i = 0; i < AssociatedNonTerminal.Rules.size();i++)
            {
                HeaderOut<<ReturnValueType<<" Parse"<<AssociatedNonTerminal.Name<<"_"<<std::to_string(i)<<"(MBCC::Tokenizer& Tokenizer);\n"; 
            }
        }
    }
    void LLParserGenerator::p_WriteSource(MBCCDefinitions const& Grammar,std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> const& ProductionsLOOk,
        std::string const& HeaderName,MBUtility::MBOctetOutputStream& SourceOut)
    {
        SourceOut<<"#include \""<<HeaderName<<"\"\n";
        p_WriteLOOKTable(ProductionsLOOk, SourceOut);
        //MBUtility::WriteData(SourceOut,"#include <MBParsing/MBCC.h>\n");
        //Order we write C/C++/C# implementations dont matter, we can just write them directly
        for(NonTerminalIndex i = 0; i < Grammar.NonTerminals.size();i++)
        {
            p_WriteNonTerminalFunction(Grammar,i,SourceOut);     
            for(int j = 0; j < Grammar.NonTerminals[i].Rules.size();j++)
            {
                p_WriteNonTerminalProduction(Grammar,i,j,"Parse"+Grammar.NonTerminals[i].Name+"_"+std::to_string(j),SourceOut); 
            }
        }       
    }
    std::string h_ColumnToBoolArray(MBMath::MBDynamicMatrix<bool> const& Matrix, int k)
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
    MBMath::MBDynamicMatrix<bool> h_CombineProductions(std::vector<MBMath::MBDynamicMatrix<bool>> const& MatrixesToCombine)
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
    //Probably the only function that needs to know about the special property that the look for all terminals are one greater than the 
    //number of non terminals, the last representing the EOF
    void LLParserGenerator::p_WriteLOOKTable(std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> const& ProductionsLOOk,MBUtility::MBOctetOutputStream& SourceOut)
    {

        //Assumes that zero non terminals is invalid
        m_ProductionPredicates.reserve(ProductionsLOOk.size());
        //for each non termina + production, for each k for each terminal
        //the k+1 production is the combined productions, representing the whole non terminal
        int TotalProductionSize = 0;
        int LOOKDepth = ProductionsLOOk[0][0].NumberOfColumns();
        //SourceOut << "const bool LOOKTable[][][][] = {";
        std::string TableString = "";
        for(auto const& NonTerminalProductions : ProductionsLOOk)
        {
            auto CombinedProductions = h_CombineProductions(NonTerminalProductions);
            TableString += "{";
            for(int k = 0; k < CombinedProductions.NumberOfColumns();k++)
            {
                TableString += h_ColumnToBoolArray(CombinedProductions,k);
                TableString += ",";
            }
            TableString += "},";
            TotalProductionSize += NonTerminalProductions.size()+1;
            for(auto const& Production : NonTerminalProductions)
            {
                TableString += "{";
                for(int k = 0; k < Production.NumberOfColumns();k++)
                {
                    TableString += h_ColumnToBoolArray(Production,k);
                    TableString += ",";
                }
                TableString += "},";
            } 
        }  
        SourceOut << "const bool LOOKTable["+std::to_string(TotalProductionSize)+"]["+std::to_string(LOOKDepth)+"]["
            +std::to_string(ProductionsLOOk[0][0].NumberOfRows())+"] = {"<<TableString<<"};\n";
        int CurrentProductionIndex = 0;
        for(auto const& CurrentProduction : ProductionsLOOk)
        {
            std::vector<std::string> NewEntry;
            NewEntry.reserve(CurrentProduction.size()+1);
            std::string CombinedString = "LOOKTable["+std::to_string(CurrentProductionIndex)+"][0][Tokenizer.Peek().Type]";
            for(int k = 1; k < LOOKDepth;k++)
            {
                CombinedString +=  "&& LOOKTable["+std::to_string(CurrentProductionIndex)+"]["+std::to_string(k)+"][Tokenizer.Peek("+std::to_string(k)+").Type]";
            }
            CurrentProductionIndex += 1;
            assert(CombinedString.size() != 0);
            NewEntry.push_back(CombinedString);
            for(int j = 0; j < CurrentProduction.size();j++)
            {
                std::string NewString = "LOOKTable["+std::to_string(CurrentProductionIndex)+"][0][Tokenizer.Peek().Type]";
                for(int k = 1; k < LOOKDepth;k++)
                {
                    NewString +=  "&& LOOKTable["+std::to_string(CurrentProductionIndex)+"]["+std::to_string(k)+"][Tokenizer.Peek("+std::to_string(k)+").Type]";
                }
                assert(NewString.size() != 0);
                NewEntry.push_back(NewString);
                CurrentProductionIndex += 1;
            }
            assert(NewEntry.size() != 0);
            m_ProductionPredicates.push_back(std::move(NewEntry));
        }
    }
    std::string const& LLParserGenerator::p_GetLOOKPredicate(NonTerminalIndex AssociatedNonTerminal,int Production)
    {
        assert(AssociatedNonTerminal < m_ProductionPredicates.size() && (Production+1 < m_ProductionPredicates[AssociatedNonTerminal].size()));
        if(Production == -1)
        {
            return(m_ProductionPredicates[AssociatedNonTerminal][0]);
        } 
        return(m_ProductionPredicates[AssociatedNonTerminal][Production+1]);
    }
    void LLParserGenerator::p_WriteNonTerminalFunction(MBCCDefinitions const& Grammar,NonTerminalIndex NonTermIndex, MBUtility::MBOctetOutputStream& SourceOut)
    {
        std::string ReturnValueType = "void";
        StructDefinition const* AssoicatedStruct = nullptr;
        NonTerminal const& AssociatedNonTerminal = Grammar.NonTerminals[NonTermIndex];
        if(AssociatedNonTerminal.AssociatedStruct != -1)
        {
            AssoicatedStruct = &Grammar.Structs[AssociatedNonTerminal.AssociatedStruct];    
        }
        if(AssoicatedStruct != nullptr)
        {
            ReturnValueType = AssoicatedStruct->Name;
        }
        MBUtility::WriteData(SourceOut,ReturnValueType+" Parse"+AssociatedNonTerminal.Name+"(MBCC::Tokenizer& Tokenizer)\n{\n");
        SourceOut<<ReturnValueType<<" ReturnValue;\n";
        for(int i = 0; i < AssociatedNonTerminal.Rules.size();i++)
        {
            if(i != 0)
            {
                MBUtility::WriteData(SourceOut,"else ");   
            }
            if(i != AssociatedNonTerminal.Rules.size()-1 || AssociatedNonTerminal.Rules.size() == 1)
            {
                MBUtility::WriteData(SourceOut,"if "); 
            }
            std::string const& LookPredicate = p_GetLOOKPredicate(NonTermIndex,i);
            assert(LookPredicate.size() != 0);
            if(AssociatedNonTerminal.Rules.size() == 1 || i + 1 < AssociatedNonTerminal.Rules.size())
            {
                MBUtility::WriteData(SourceOut,"("+LookPredicate+")\n{\n");    
            }
            else
            {
                SourceOut << "\n{\n";
            }
            if(AssoicatedStruct != nullptr)
            {
                MBUtility::WriteData(SourceOut,"ReturnValue = ");    
            }
            MBUtility::WriteData(SourceOut,"Parse"+AssociatedNonTerminal.Name+"_"+std::to_string(i)+"(Tokenizer);\n}\n");
        }
        if(AssoicatedStruct != nullptr)
        {
            MBUtility::WriteData(SourceOut,"return(ReturnValue);\n}\n");
        }
        else
        {
            MBUtility::WriteData(SourceOut,"}\n");
        }
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
    void LLParserGenerator::p_WriteNonTerminalProduction(MBCCDefinitions const& Grammar,NonTerminalIndex NonTermIndex,int ProductionIndex,std::string const& FunctionName,MBUtility::MBOctetOutputStream& SourceOut)
    {
        std::string ReturnValueType = "void";
        StructDefinition const* AssoicatedStruct = nullptr;
        NonTerminal const& AssociatedNonTerminal = Grammar.NonTerminals[NonTermIndex];
        ParseRule const& Production = AssociatedNonTerminal.Rules[ProductionIndex];
        if(AssociatedNonTerminal.AssociatedStruct != -1)
        {
            AssoicatedStruct = &Grammar.Structs[AssociatedNonTerminal.AssociatedStruct];    
        }
        if(AssoicatedStruct != nullptr)
        {
            ReturnValueType = AssoicatedStruct->Name;
        }
        MBUtility::WriteData(SourceOut,ReturnValueType+" "+FunctionName+"(MBCC::Tokenizer& Tokenizer)\n{\n");
        if(AssociatedNonTerminal.AssociatedStruct != -1)
        {
            MBUtility::WriteData(SourceOut,AssoicatedStruct->Name + " ReturnValue;\n");
        }
        std::vector<std::string> DelayedAssignments;
        for(auto const& Component : Production.Components)
        {
            std::string AssignPrefix;
            if(Component.AssignedMember != "")
            {
                if(Component.AssignedMember == "this")
                {
                    AssignPrefix = "ReturnValue = ";   
                }
                else
                {
                    if(Production.NeedsAssignmentOrder)
                    {
                        SourceOut <<h_GetTypeCppTypeString(Grammar.GetMember(*AssoicatedStruct,Component.AssignedMember))<<" "<<Component.AssignedMember<<";\n";    
                        AssignPrefix = Component.AssignedMember;
                        DelayedAssignments.push_back(Component.AssignedMember);
                    }
                    else
                    {
                        AssignPrefix = "ReturnValue"; 
                    }
                    if(AssociatedNonTerminal.AssociatedStruct != -1 && Grammar.DepInfo.ChildrenMap[AssociatedNonTerminal.AssociatedStruct].size() > 0 && !Production.NeedsAssignmentOrder)
                    {
                        AssignPrefix += ".GetBase()";
                    }
                    if(!Production.NeedsAssignmentOrder)
                    {
                        AssignPrefix += "."+Component.AssignedMember; 
                    }
                    if(Component.Max == -1)
                    {
                         AssignPrefix += ".push_back(";   
                    }
                    else
                    {
                         AssignPrefix += " = ";   
                    }
                }
            }
            if(Component.IsTerminal)
            {
                if (Component.Min == 1 && Component.Max == 1) {
                    MBUtility::WriteData(SourceOut, "if(Tokenizer.Peek().Type != " + std::to_string(Component.ComponentIndex) + ")\n{\nthrow std::runtime_error(\"Error parsing " + AssociatedNonTerminal.Name + ": expected " + Grammar.Terminals[Component.ComponentIndex].Name
                        + "\");\n}\n");
                }
                else 
                {
                    if (Component.Max == 1 && Component.Min == 0)
                    {
                        SourceOut << "if(Tokenizer.Peek().Type == " << std::to_string(Component.ComponentIndex) << ")\n{\n";
                    }
                    else if (Component.Max == -1)
                    {
                        SourceOut << "while(Tokenizer.Peek().Type == " << std::to_string(Component.ComponentIndex) << ")\n{\n";
                    }
                }
                if(Component.AssignedMember != "")
                {
                    MBUtility::WriteData(SourceOut,AssignPrefix);
                    StructMemberVariable const& Member = Grammar.GetMember(*AssoicatedStruct,Component.AssignedMember);
                    if(Member.IsType<StructMemberVariable_String>())
                    {
                        MBUtility::WriteData(SourceOut,"Tokenizer.Peek().Value;\n");
                    }
                    else if(Member.IsType<StructMemberVariable_Int>())
                    {
                        MBUtility::WriteData(SourceOut,"std::stoi(Tokenizer.Peek().Value);\n");
                    }
                    else if(Member.IsType<StructMemberVariable_Bool>())
                    {
                        MBUtility::WriteData(SourceOut,"Tokenizer.Peek().Value == \"true\";\n");
                    }
                } 
                MBUtility::WriteData(SourceOut,"Tokenizer.ConsumeToken();\n");
                if (!(Component.Max == 1 && Component.Min == 1))
                {
                    SourceOut << "\n}\n";
                }
            } 
            else
            {
                if(Component.Min == 1 && Component.Max == 1)
                {
                    SourceOut<<AssignPrefix;
                    MBUtility::WriteData(SourceOut,"Parse"+Grammar.NonTerminals[Component.ComponentIndex].Name+"(Tokenizer);\n");
                } 
                else if(Component.Min == 0 && Component.Max == -1)
                {
                    MBUtility::WriteData(SourceOut,"while("+p_GetLOOKPredicate(Component.ComponentIndex)+")\n{\n");
                    if(Component.AssignedMember != "")
                    {
                        SourceOut<<AssignPrefix;
                        MBUtility::WriteData(SourceOut,"Parse"+Grammar.NonTerminals[Component.ComponentIndex].Name+"(Tokenizer));\n}\n");
                    }
                    else
                    {
                        MBUtility::WriteData(SourceOut,"Parse"+Grammar.NonTerminals[Component.ComponentIndex].Name+"(Tokenizer);\n}\n");
                    }
                }
                else if(Component.Min == 0 && Component.Max == 1)
                {
                    MBUtility::WriteData(SourceOut,"if("+p_GetLOOKPredicate(Component.ComponentIndex)+")\n{\n");
                    if(Component.AssignedMember != "")
                    {
                        //NOTE: multi variable member cannot be assigned to this
                        SourceOut<<AssignPrefix;
                    }
                    MBUtility::WriteData(SourceOut,"Parse"+Grammar.NonTerminals[Component.ComponentIndex].Name+"(Tokenizer);\n}\n");
                }
                else
                {
                    assert(false && "Min can only be 0 or 1, max can only be 1 or -1");
                }
            }
        }
        for(auto const& MemberName : DelayedAssignments)
        {
            SourceOut<<"ReturnValue.";    
            if(AssociatedNonTerminal.AssociatedStruct != -1 && Grammar.DepInfo.ChildrenMap[AssociatedNonTerminal.AssociatedStruct].size() > 0)
            {
                SourceOut<<"GetBase()."; 
            }
            SourceOut<<MemberName<<" = std::move("<<MemberName<<");\n";
        }
        for(auto const& Action : Grammar.NonTerminals[NonTermIndex].Rules[ProductionIndex].Actions)
        {
            SourceOut<<Action.ActionString<<"\n";
        }
        MBUtility::WriteData(SourceOut,"return(ReturnValue);\n}\n");
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
