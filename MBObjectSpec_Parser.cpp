#include "MBObjectSpec_Parser.h"
const bool LOOKTable[5][1][9] = {{{false,false,false,false,false,false,false,true,false,},},{{false,false,false,false,false,false,false,true,false,},},{{false,false,false,false,true,false,true,false,false,},},{{false,false,false,false,false,false,true,false,false,},},{{false,false,false,false,true,false,false,false,false,},},};
StructDefinition ParseStructDefinition(MBCC::Tokenizer& Tokenizer)
{
    StructDefinition ReturnValue;
    if (LOOKTable[1][0][Tokenizer.Peek().Type])
    {
        ReturnValue = ParseStructDefinition_0(Tokenizer);
    }
    return(ReturnValue);
}
StructDefinition ParseStructDefinition_0(MBCC::Tokenizer& Tokenizer)
{
    StructDefinition ReturnValue;
    if(Tokenizer.Peek().Type != 7)
    {
        throw std::runtime_error("Error parsing StructDefinition: expected struct");
    }
    Tokenizer.ConsumeToken();
    if(Tokenizer.Peek().Type != 6)
    {
        throw std::runtime_error("Error parsing StructDefinition: expected idf");
    }
    Tokenizer.ConsumeToken();
    if(Tokenizer.Peek().Type != 2)
    {
        throw std::runtime_error("Error parsing StructDefinition: expected lcurl");
    }
    Tokenizer.ConsumeToken();
    while(LOOKTable[2][0][Tokenizer.Peek().Type])
    {ReturnValue.Members.push_back(ParseMemberDefinition(Tokenizer));
    }
    if(Tokenizer.Peek().Type != 3)
    {
        throw std::runtime_error("Error parsing StructDefinition: expected rcurl");
    }
    Tokenizer.ConsumeToken();
    return(ReturnValue);
}
MemberDefinition ParseMemberDefinition(MBCC::Tokenizer& Tokenizer)
{
    MemberDefinition ReturnValue;
    if (LOOKTable[3][0][Tokenizer.Peek().Type])
    {
        ReturnValue = ParseMemberDefinition_0(Tokenizer);
    }
    else 
    {
        ReturnValue = ParseMemberDefinition_1(Tokenizer);
    }
    return(ReturnValue);
}
MemberDefinition ParseMemberDefinition_0(MBCC::Tokenizer& Tokenizer)
{
    MemberDefinition ReturnValue;
    if(Tokenizer.Peek().Type != 6)
    {
        throw std::runtime_error("Error parsing MemberDefinition: expected idf");
    }
    ReturnValue.Type= Tokenizer.Peek().Value;
    Tokenizer.ConsumeToken();
    if(Tokenizer.Peek().Type != 6)
    {
        throw std::runtime_error("Error parsing MemberDefinition: expected idf");
    }
    ReturnValue.Name= Tokenizer.Peek().Value;
    Tokenizer.ConsumeToken();
    if(Tokenizer.Peek().Type != 8)
    {
        throw std::runtime_error("Error parsing MemberDefinition: expected semicolon");
    }
    Tokenizer.ConsumeToken();
    return(ReturnValue);
}
MemberDefinition ParseMemberDefinition_1(MBCC::Tokenizer& Tokenizer)
{
    MemberDefinition ReturnValue;
    if(Tokenizer.Peek().Type != 4)
    {
        throw std::runtime_error("Error parsing MemberDefinition: expected lsquare");
    }
    Tokenizer.ConsumeToken();
    if(Tokenizer.Peek().Type != 6)
    {
        throw std::runtime_error("Error parsing MemberDefinition: expected idf");
    }
    ReturnValue.Type= Tokenizer.Peek().Value;
    Tokenizer.ConsumeToken();
    if(Tokenizer.Peek().Type != 6)
    {
        throw std::runtime_error("Error parsing MemberDefinition: expected idf");
    }
    ReturnValue.Name= Tokenizer.Peek().Value;
    Tokenizer.ConsumeToken();
    if(Tokenizer.Peek().Type != 8)
    {
        throw std::runtime_error("Error parsing MemberDefinition: expected semicolon");
    }
    Tokenizer.ConsumeToken();
    if(Tokenizer.Peek().Type != 5)
    {
        throw std::runtime_error("Error parsing MemberDefinition: expected rsquare");
    }
    Tokenizer.ConsumeToken();
    return(ReturnValue);
}
