#pragma once
#include <MBCC/MBCC.h>
#include <string>
#include <vector>
#include <stdexcept>
class MemberDefinition
{
    public:
    std::string Type =  "";
    std::string Name =  "";
    
};
class StructDefinition
{
    public:
    std::vector<MemberDefinition> Members;
    
};
StructDefinition ParseStructDefinition(MBCC::Tokenizer& Tokenizer);
StructDefinition ParseStructDefinition_0(MBCC::Tokenizer& Tokenizer);
MemberDefinition ParseMemberDefinition(MBCC::Tokenizer& Tokenizer);
MemberDefinition ParseMemberDefinition_0(MBCC::Tokenizer& Tokenizer);
MemberDefinition ParseMemberDefinition_1(MBCC::Tokenizer& Tokenizer);
inline MBCC::Tokenizer GetTokenizer()
{
    MBCC::Tokenizer ReturnValue(" \t\n",{"(",")","{","}","[","]","[[:alpha:]_]+[[:alphanum:]_]*","struct",";",});
    return(ReturnValue);
}