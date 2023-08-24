#include "Test.h"
const bool LOOKTable[42][2][22] = {{{false,false,false,false,false,false,false,false,false,false,false,true,true,true,false,false,false,false,false,false,false,false,},{false,false,false,false,false,false,false,true,false,false,true,false,false,false,false,false,false,false,false,false,false,true,},},{{false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,false,},{false,false,false,false,false,false,false,true,false,false,true,false,false,false,false,false,false,false,false,false,false,true,},},{{false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,false,false,false,},{false,false,false,false,false,false,false,true,false,false,true,false,false,false,false,false,false,false,false,false,false,true,},},{{false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,false,false,},{false,false,false,false,false,false,false,true,false,false,true,false,false,false,false,false,false,false,false,false,false,true,},},{{false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,false,},{false,false,false,false,false,false,false,true,false,false,true,false,false,false,false,false,false,false,false,false,false,true,},},{{false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,false,},{false,false,false,false,false,false,false,true,false,false,true,false,false,false,false,false,false,false,false,false,false,true,},},{{false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,false,false,false,},{false,false,false,false,false,false,false,true,false,false,true,false,false,false,false,false,false,false,false,false,false,true,},},{{false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,false,false,false,},{false,false,false,false,false,false,false,true,false,false,true,false,false,false,false,false,false,false,false,false,false,true,},},{{false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,false,false,},{false,false,false,false,false,false,false,true,false,false,true,false,false,false,false,false,false,false,false,false,false,true,},},{{false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,false,false,},{false,false,false,false,false,false,false,true,false,false,true,false,false,false,false,false,false,false,false,false,false,true,},},{{false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,},{false,false,false,false,false,false,false,false,false,false,false,true,true,true,false,false,false,false,false,false,false,false,},},{{false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,},{false,false,false,false,false,false,false,false,false,false,false,true,true,true,false,false,false,false,false,false,false,false,},},{{false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,},{false,false,false,false,false,false,true,false,false,false,false,false,false,false,true,false,false,false,true,true,false,true,},},{{false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,},{false,false,false,false,false,false,true,false,false,false,false,false,false,false,true,false,false,false,true,false,false,true,},},{{false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,},{false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,},},{{false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,},{false,false,false,false,false,false,true,false,false,false,false,false,false,false,true,false,false,false,true,false,false,true,},},{{false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,},{false,false,false,false,false,false,true,false,false,false,false,false,false,false,true,false,false,false,true,false,false,true,},},{{false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,},{false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,},},{{false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,},{false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,},},{{false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,},{false,false,false,false,false,false,true,false,false,false,false,false,false,false,true,false,false,false,true,true,false,true,},},{{false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,},{false,false,false,false,false,false,true,false,false,false,false,false,false,false,true,false,false,false,true,true,false,true,},},{{false,false,false,false,true,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,},{false,false,false,false,false,false,true,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,},},{{false,false,false,false,true,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,},{false,false,false,false,false,false,true,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,},},{{false,false,false,false,false,false,false,false,false,true,false,false,false,false,true,false,false,false,false,false,false,false,},{false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,true,false,false,false,false,false,false,},},{{false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,},{false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,},},{{false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,false,false,false,false,false,},{false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,},},{{false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,},{false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,},},{{false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,},{false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,},},{{false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,false,false,false,false,false,},{false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,},},{{false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,false,false,false,false,false,},{false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,},},{{false,false,false,false,false,true,false,false,false,true,false,false,false,false,true,false,false,false,false,false,true,false,},{false,false,false,false,false,false,false,false,false,true,false,false,false,false,true,true,false,false,false,false,false,false,},},{{false,false,false,false,false,false,false,false,false,true,false,false,false,false,true,false,false,false,false,false,false,false,},{false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,true,false,false,false,false,false,false,},},{{false,false,false,false,false,true,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,},{false,false,false,false,false,false,false,false,false,true,false,false,false,false,true,false,false,false,false,false,false,false,},},{{false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,},{false,false,false,false,false,false,false,false,false,true,false,false,false,false,true,false,false,false,false,false,false,false,},},{{false,false,false,false,true,true,false,false,false,true,false,false,false,false,true,false,false,false,false,false,true,false,},{false,false,false,false,false,false,true,false,false,true,false,false,false,false,true,true,false,false,false,false,false,false,},},{{false,false,false,false,true,true,false,false,false,true,false,false,false,false,true,false,false,false,false,false,true,false,},{false,false,false,false,false,false,true,false,false,true,false,false,false,false,true,true,false,false,false,false,false,false,},},{{false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,},{false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,},},{{false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,},{false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,},},{{false,false,false,false,true,false,false,false,true,false,false,false,false,false,false,false,false,false,false,false,false,false,},{false,false,false,false,false,false,true,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,},},{{false,false,false,false,true,false,false,false,true,false,false,false,false,false,false,false,false,false,false,false,false,false,},{false,false,false,false,false,false,true,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,},},{{false,false,false,false,true,false,false,false,true,false,false,false,false,false,false,false,false,false,false,false,false,true,},{false,false,false,false,false,false,true,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,},},{{false,false,false,false,true,false,false,false,true,false,false,false,false,false,false,false,false,false,false,false,false,true,},{false,false,false,false,false,false,true,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,},},};
Literal ParseLiteral(MBCC::Tokenizer& Tokenizer)
{
    Literal ReturnValue;
    if (LOOKTable[1][0][Tokenizer.Peek().Type]&& LOOKTable[1][1][Tokenizer.Peek(1).Type])
    {
        ReturnValue = ParseLiteral_0(Tokenizer);
    }
    else if (LOOKTable[2][0][Tokenizer.Peek().Type]&& LOOKTable[2][1][Tokenizer.Peek(1).Type])
    {
        ReturnValue = ParseLiteral_1(Tokenizer);
    }
    else if (LOOKTable[3][0][Tokenizer.Peek().Type]&& LOOKTable[3][1][Tokenizer.Peek(1).Type])
    {
        ReturnValue = ParseLiteral_2(Tokenizer);
    }
    else
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"Literal","Literal");
        
    }
    return(ReturnValue);
}
Literal ParseLiteral_0(MBCC::Tokenizer& Tokenizer)
{
    Literal ReturnValue;
    if(!(LOOKTable[4][0][Tokenizer.Peek().Type]&& LOOKTable[4][1][Tokenizer.Peek(1).Type]))
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"Literal","RPar");
        
    }
    ReturnValue = ParseLiteral_Bool(Tokenizer);
    return(ReturnValue);
}
Literal ParseLiteral_1(MBCC::Tokenizer& Tokenizer)
{
    Literal ReturnValue;
    if(!(LOOKTable[6][0][Tokenizer.Peek().Type]&& LOOKTable[6][1][Tokenizer.Peek(1).Type]))
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"Literal","lcurl");
        
    }
    ReturnValue = ParseLiteral_Number(Tokenizer);
    return(ReturnValue);
}
Literal ParseLiteral_2(MBCC::Tokenizer& Tokenizer)
{
    Literal ReturnValue;
    if(!(LOOKTable[8][0][Tokenizer.Peek().Type]&& LOOKTable[8][1][Tokenizer.Peek(1).Type]))
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"Literal","rcurl");
        
    }
    ReturnValue = ParseLiteral_String(Tokenizer);
    return(ReturnValue);
}
Literal_Bool ParseLiteral_Bool(MBCC::Tokenizer& Tokenizer)
{
    Literal_Bool ReturnValue;
    ReturnValue = ParseLiteral_Bool_0(Tokenizer);
    return(ReturnValue);
}
Literal_Bool ParseLiteral_Bool_0(MBCC::Tokenizer& Tokenizer)
{
    Literal_Bool ReturnValue;
    if(Tokenizer.Peek().Type != 13)
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"Literal_Bool","bool");
        
    }
    ReturnValue.Value = Tokenizer.Peek().Value == "true";
    Tokenizer.ConsumeToken();
    return(ReturnValue);
}
Literal_Number ParseLiteral_Number(MBCC::Tokenizer& Tokenizer)
{
    Literal_Number ReturnValue;
    ReturnValue = ParseLiteral_Number_0(Tokenizer);
    return(ReturnValue);
}
Literal_Number ParseLiteral_Number_0(MBCC::Tokenizer& Tokenizer)
{
    Literal_Number ReturnValue;
    if(Tokenizer.Peek().Type != 11)
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"Literal_Number","number");
        
    }
    ReturnValue.Value = std::stoi(Tokenizer.Peek().Value);
    Tokenizer.ConsumeToken();
    return(ReturnValue);
}
Literal_String ParseLiteral_String(MBCC::Tokenizer& Tokenizer)
{
    Literal_String ReturnValue;
    ReturnValue = ParseLiteral_String_0(Tokenizer);
    return(ReturnValue);
}
Literal_String ParseLiteral_String_0(MBCC::Tokenizer& Tokenizer)
{
    Literal_String ReturnValue;
    if(Tokenizer.Peek().Type != 12)
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"Literal_String","string");
        
    }
    ReturnValue.Value = Tokenizer.Peek().Value;
    Tokenizer.ConsumeToken();
    return(ReturnValue);
}
DefaultValue ParseDefaultValue(MBCC::Tokenizer& Tokenizer)
{
    DefaultValue ReturnValue;
    ReturnValue = ParseDefaultValue_0(Tokenizer);
    return(ReturnValue);
}
DefaultValue ParseDefaultValue_0(MBCC::Tokenizer& Tokenizer)
{
    DefaultValue ReturnValue;
    if(Tokenizer.Peek().Type != 19)
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"DefaultValue","eq");
        
    }
    Tokenizer.ConsumeToken();
    if(!(LOOKTable[0][0][Tokenizer.Peek().Type]&& LOOKTable[0][1][Tokenizer.Peek(1).Type]))
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"DefaultValue","LPar");
        
    }
    ReturnValue.Value = ParseLiteral(Tokenizer);
    return(ReturnValue);
}
Attribute ParseAttribute(MBCC::Tokenizer& Tokenizer)
{
    Attribute ReturnValue;
    if (LOOKTable[13][0][Tokenizer.Peek().Type]&& LOOKTable[13][1][Tokenizer.Peek(1).Type])
    {
        ReturnValue = ParseAttribute_0(Tokenizer);
    }
    else if (LOOKTable[14][0][Tokenizer.Peek().Type]&& LOOKTable[14][1][Tokenizer.Peek(1).Type])
    {
        ReturnValue = ParseAttribute_1(Tokenizer);
    }
    else
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"Attribute","Attribute");
        
    }
    return(ReturnValue);
}
Attribute ParseAttribute_0(MBCC::Tokenizer& Tokenizer)
{
    Attribute ReturnValue;
    if(!(LOOKTable[15][0][Tokenizer.Peek().Type]&& LOOKTable[15][1][Tokenizer.Peek(1).Type]))
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"Attribute","drsquare");
        
    }
    ParseAttribute_Flag(Tokenizer);
    return(ReturnValue);
}
Attribute ParseAttribute_1(MBCC::Tokenizer& Tokenizer)
{
    Attribute ReturnValue;
    if(!(LOOKTable[17][0][Tokenizer.Peek().Type]&& LOOKTable[17][1][Tokenizer.Peek(1).Type]))
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"Attribute","rsquare");
        
    }
    ParseAttribute_Value(Tokenizer);
    return(ReturnValue);
}
Attribute_Flag ParseAttribute_Flag(MBCC::Tokenizer& Tokenizer)
{
    Attribute_Flag ReturnValue;
    ReturnValue = ParseAttribute_Flag_0(Tokenizer);
    return(ReturnValue);
}
Attribute_Flag ParseAttribute_Flag_0(MBCC::Tokenizer& Tokenizer)
{
    Attribute_Flag ReturnValue;
    if(Tokenizer.Peek().Type != 14)
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"Attribute_Flag","idf");
        
    }
    ReturnValue.Name = Tokenizer.Peek().Value;
    Tokenizer.ConsumeToken();
    return(ReturnValue);
}
Attribute_Value ParseAttribute_Value(MBCC::Tokenizer& Tokenizer)
{
    Attribute_Value ReturnValue;
    ReturnValue = ParseAttribute_Value_0(Tokenizer);
    return(ReturnValue);
}
Attribute_Value ParseAttribute_Value_0(MBCC::Tokenizer& Tokenizer)
{
    Attribute_Value ReturnValue;
    if(Tokenizer.Peek().Type != 14)
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"Attribute_Value","idf");
        
    }
    ReturnValue.Name = Tokenizer.Peek().Value;
    Tokenizer.ConsumeToken();
    if(Tokenizer.Peek().Type != 19)
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"Attribute_Value","eq");
        
    }
    Tokenizer.ConsumeToken();
    if(Tokenizer.Peek().Type != 12)
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"Attribute_Value","string");
        
    }
    ReturnValue.Value = Tokenizer.Peek().Value;
    Tokenizer.ConsumeToken();
    return(ReturnValue);
}
Attribute ParseAttributeArgument(MBCC::Tokenizer& Tokenizer)
{
    Attribute ReturnValue;
    ReturnValue = ParseAttributeArgument_0(Tokenizer);
    return(ReturnValue);
}
Attribute ParseAttributeArgument_0(MBCC::Tokenizer& Tokenizer)
{
    Attribute ReturnValue;
    if(!(LOOKTable[12][0][Tokenizer.Peek().Type]&& LOOKTable[12][1][Tokenizer.Peek(1).Type]))
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"AttributeArgument","lsquare");
        
    }
    ReturnValue = ParseAttribute(Tokenizer);
    if(Tokenizer.Peek().Type == 18)
    {
        Tokenizer.ConsumeToken();
        
    }
    return(ReturnValue);
}
AttributeList ParseAttributeList(MBCC::Tokenizer& Tokenizer)
{
    AttributeList ReturnValue;
    ReturnValue = ParseAttributeList_0(Tokenizer);
    return(ReturnValue);
}
AttributeList ParseAttributeList_0(MBCC::Tokenizer& Tokenizer)
{
    AttributeList ReturnValue;
    if(Tokenizer.Peek().Type != 4)
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"AttributeList","dlsquare");
        
    }
    Tokenizer.ConsumeToken();
    while(LOOKTable[19][0][Tokenizer.Peek().Type]&& LOOKTable[19][1][Tokenizer.Peek(1).Type])
    {
        ReturnValue.Attributes.push_back(ParseAttributeArgument(Tokenizer));
        
    }
    if(Tokenizer.Peek().Type != 6)
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"AttributeList","drsquare");
        
    }
    Tokenizer.ConsumeToken();
    return(ReturnValue);
}
MemberDefinition ParseMemberDefinition_Base(MBCC::Tokenizer& Tokenizer)
{
    MemberDefinition ReturnValue;
    if (LOOKTable[24][0][Tokenizer.Peek().Type]&& LOOKTable[24][1][Tokenizer.Peek(1).Type])
    {
        ReturnValue = ParseMemberDefinition_Base_0(Tokenizer);
    }
    else if (LOOKTable[25][0][Tokenizer.Peek().Type]&& LOOKTable[25][1][Tokenizer.Peek(1).Type])
    {
        ReturnValue = ParseMemberDefinition_Base_1(Tokenizer);
    }
    else
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"MemberDefinition_Base","MemberDefinition_Base");
        
    }
    return(ReturnValue);
}
MemberDefinition ParseMemberDefinition_Base_0(MBCC::Tokenizer& Tokenizer)
{
    MemberDefinition ReturnValue;
    if(!(LOOKTable[26][0][Tokenizer.Peek().Type]&& LOOKTable[26][1][Tokenizer.Peek(1).Type]))
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"MemberDefinition_Base","number");
        
    }
    ReturnValue = ParseMemberDefinition_Struct(Tokenizer);
    return(ReturnValue);
}
MemberDefinition ParseMemberDefinition_Base_1(MBCC::Tokenizer& Tokenizer)
{
    MemberDefinition ReturnValue;
    if(!(LOOKTable[28][0][Tokenizer.Peek().Type]&& LOOKTable[28][1][Tokenizer.Peek(1).Type]))
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"MemberDefinition_Base","string");
        
    }
    ReturnValue = ParseMemberDefinition_List(Tokenizer);
    return(ReturnValue);
}
MemberDefinition_Struct ParseMemberDefinition_Struct(MBCC::Tokenizer& Tokenizer)
{
    MemberDefinition_Struct ReturnValue;
    ReturnValue = ParseMemberDefinition_Struct_0(Tokenizer);
    return(ReturnValue);
}
MemberDefinition_Struct ParseMemberDefinition_Struct_0(MBCC::Tokenizer& Tokenizer)
{
    MemberDefinition_Struct ReturnValue;
    if(Tokenizer.Peek().Type != 14)
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"MemberDefinition_Struct","idf");
        
    }
    ReturnValue.StructType = Tokenizer.Peek().Value;
    Tokenizer.ConsumeToken();
    if(Tokenizer.Peek().Type != 14)
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"MemberDefinition_Struct","idf");
        
    }
    ReturnValue.Name = Tokenizer.Peek().Value;
    Tokenizer.ConsumeToken();
    return(ReturnValue);
}
MemberDefinition_List ParseMemberDefinition_List(MBCC::Tokenizer& Tokenizer)
{
    MemberDefinition_List ReturnValue;
    ReturnValue = ParseMemberDefinition_List_0(Tokenizer);
    return(ReturnValue);
}
MemberDefinition_List ParseMemberDefinition_List_0(MBCC::Tokenizer& Tokenizer)
{
    MemberDefinition_List ReturnValue;
    if(Tokenizer.Peek().Type != 9)
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"MemberDefinition_List","list");
        
    }
    Tokenizer.ConsumeToken();
    if(Tokenizer.Peek().Type != 15)
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"MemberDefinition_List","less");
        
    }
    Tokenizer.ConsumeToken();
    if(Tokenizer.Peek().Type != 14)
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"MemberDefinition_List","idf");
        
    }
    ReturnValue.ListType = Tokenizer.Peek().Value;
    Tokenizer.ConsumeToken();
    if(Tokenizer.Peek().Type != 16)
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"MemberDefinition_List","great");
        
    }
    Tokenizer.ConsumeToken();
    if(Tokenizer.Peek().Type != 14)
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"MemberDefinition_List","idf");
        
    }
    ReturnValue.Name = Tokenizer.Peek().Value;
    Tokenizer.ConsumeToken();
    return(ReturnValue);
}
MemberDefinition ParseMemberDefinition_Member(MBCC::Tokenizer& Tokenizer)
{
    MemberDefinition ReturnValue;
    if (LOOKTable[31][0][Tokenizer.Peek().Type]&& LOOKTable[31][1][Tokenizer.Peek(1).Type])
    {
        ReturnValue = ParseMemberDefinition_Member_0(Tokenizer);
    }
    else if (LOOKTable[32][0][Tokenizer.Peek().Type]&& LOOKTable[32][1][Tokenizer.Peek(1).Type])
    {
        ReturnValue = ParseMemberDefinition_Member_1(Tokenizer);
    }
    else if (LOOKTable[33][0][Tokenizer.Peek().Type]&& LOOKTable[33][1][Tokenizer.Peek(1).Type])
    {
        ReturnValue = ParseMemberDefinition_Member_2(Tokenizer);
    }
    else
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"MemberDefinition_Member","MemberDefinition_Member");
        
    }
    return(ReturnValue);
}
MemberDefinition ParseMemberDefinition_Member_0(MBCC::Tokenizer& Tokenizer)
{
    MemberDefinition ReturnValue;
    if(!(LOOKTable[23][0][Tokenizer.Peek().Type]&& LOOKTable[23][1][Tokenizer.Peek(1).Type]))
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"MemberDefinition_Member","semicolon");
        
    }
    ReturnValue = ParseMemberDefinition_Base(Tokenizer);
    DefaultValue MBCC_TempVar0;
    if(LOOKTable[10][0][Tokenizer.Peek().Type]&& LOOKTable[10][1][Tokenizer.Peek(1).Type])
    {
        MBCC_TempVar0 = ParseDefaultValue(Tokenizer);
        
    }
    if(Tokenizer.Peek().Type != 10)
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"MemberDefinition_Member","semicolon");
        
    }
    Tokenizer.ConsumeToken();
    ReturnValue.GetBase().DefValue = std::move(MBCC_TempVar0);
    return(ReturnValue);
}
MemberDefinition ParseMemberDefinition_Member_1(MBCC::Tokenizer& Tokenizer)
{
    MemberDefinition ReturnValue;
    if(Tokenizer.Peek().Type != 5)
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"MemberDefinition_Member","lsquare");
        
    }
    Tokenizer.ConsumeToken();
    if(!(LOOKTable[23][0][Tokenizer.Peek().Type]&& LOOKTable[23][1][Tokenizer.Peek(1).Type]))
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"MemberDefinition_Member","semicolon");
        
    }
    ReturnValue = ParseMemberDefinition_Base(Tokenizer);
    DefaultValue MBCC_TempVar1;
    if(LOOKTable[10][0][Tokenizer.Peek().Type]&& LOOKTable[10][1][Tokenizer.Peek(1).Type])
    {
        MBCC_TempVar1 = ParseDefaultValue(Tokenizer);
        
    }
    if(Tokenizer.Peek().Type != 7)
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"MemberDefinition_Member","rsquare");
        
    }
    Tokenizer.ConsumeToken();
    if(Tokenizer.Peek().Type != 10)
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"MemberDefinition_Member","semicolon");
        
    }
    Tokenizer.ConsumeToken();
    ReturnValue.GetBase().DefValue = std::move(MBCC_TempVar1);
    ReturnValue.GetBase().Optional = true;
    return(ReturnValue);
}
MemberDefinition ParseMemberDefinition_Member_2(MBCC::Tokenizer& Tokenizer)
{
    MemberDefinition ReturnValue;
    if(Tokenizer.Peek().Type != 20)
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"MemberDefinition_Member","que");
        
    }
    Tokenizer.ConsumeToken();
    if(!(LOOKTable[23][0][Tokenizer.Peek().Type]&& LOOKTable[23][1][Tokenizer.Peek(1).Type]))
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"MemberDefinition_Member","semicolon");
        
    }
    ReturnValue = ParseMemberDefinition_Base(Tokenizer);
    DefaultValue MBCC_TempVar2;
    if(LOOKTable[10][0][Tokenizer.Peek().Type]&& LOOKTable[10][1][Tokenizer.Peek(1).Type])
    {
        MBCC_TempVar2 = ParseDefaultValue(Tokenizer);
        
    }
    if(Tokenizer.Peek().Type != 10)
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"MemberDefinition_Member","semicolon");
        
    }
    Tokenizer.ConsumeToken();
    ReturnValue.GetBase().DefValue = std::move(MBCC_TempVar2);
    ReturnValue.GetBase().Omittable = true;
    return(ReturnValue);
}
MemberDefinition ParseMemberDefinition(MBCC::Tokenizer& Tokenizer)
{
    MemberDefinition ReturnValue;
    ReturnValue = ParseMemberDefinition_0(Tokenizer);
    return(ReturnValue);
}
MemberDefinition ParseMemberDefinition_0(MBCC::Tokenizer& Tokenizer)
{
    MemberDefinition ReturnValue;
    AttributeList MBCC_TempVar3;
    if(LOOKTable[21][0][Tokenizer.Peek().Type]&& LOOKTable[21][1][Tokenizer.Peek(1).Type])
    {
        MBCC_TempVar3 = ParseAttributeList(Tokenizer);
        
    }
    if(!(LOOKTable[30][0][Tokenizer.Peek().Type]&& LOOKTable[30][1][Tokenizer.Peek(1).Type]))
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"MemberDefinition","bool");
        
    }
    ReturnValue = ParseMemberDefinition_Member(Tokenizer);
    ReturnValue.GetBase().Attributes = std::move(MBCC_TempVar3);
    return(ReturnValue);
}
Inheritance ParseInheritance(MBCC::Tokenizer& Tokenizer)
{
    Inheritance ReturnValue;
    ReturnValue = ParseInheritance_0(Tokenizer);
    return(ReturnValue);
}
Inheritance ParseInheritance_0(MBCC::Tokenizer& Tokenizer)
{
    Inheritance ReturnValue;
    if(Tokenizer.Peek().Type != 17)
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"Inheritance","colon");
        
    }
    Tokenizer.ConsumeToken();
    if(Tokenizer.Peek().Type != 14)
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"Inheritance","idf");
        
    }
    ReturnValue.Name = Tokenizer.Peek().Value;
    Tokenizer.ConsumeToken();
    return(ReturnValue);
}
StructDefinition ParseStructDefinition(MBCC::Tokenizer& Tokenizer)
{
    StructDefinition ReturnValue;
    ReturnValue = ParseStructDefinition_0(Tokenizer);
    return(ReturnValue);
}
StructDefinition ParseStructDefinition_0(MBCC::Tokenizer& Tokenizer)
{
    StructDefinition ReturnValue;
    if(LOOKTable[21][0][Tokenizer.Peek().Type]&& LOOKTable[21][1][Tokenizer.Peek(1).Type])
    {
        ReturnValue.Attributes = ParseAttributeList(Tokenizer);
        
    }
    if(Tokenizer.Peek().Type != 8)
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"StructDefinition","struct");
        
    }
    Tokenizer.ConsumeToken();
    if(Tokenizer.Peek().Type != 14)
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"StructDefinition","idf");
        
    }
    ReturnValue.Name = Tokenizer.Peek().Value;
    Tokenizer.ConsumeToken();
    if(LOOKTable[36][0][Tokenizer.Peek().Type]&& LOOKTable[36][1][Tokenizer.Peek(1).Type])
    {
        ReturnValue.Parent = ParseInheritance(Tokenizer);
        
    }
    if(Tokenizer.Peek().Type != 2)
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"StructDefinition","lcurl");
        
    }
    Tokenizer.ConsumeToken();
    while(LOOKTable[34][0][Tokenizer.Peek().Type]&& LOOKTable[34][1][Tokenizer.Peek(1).Type])
    {
        ReturnValue.Members.push_back(ParseMemberDefinition(Tokenizer));
        
    }
    if(Tokenizer.Peek().Type != 3)
    {
        throw MBCC::ParsingException(Tokenizer.Peek().Position,"StructDefinition","rcurl");
        
    }
    Tokenizer.ConsumeToken();
    return(ReturnValue);
}
StructList ParseStructList(MBCC::Tokenizer& Tokenizer)
{
    StructList ReturnValue;
    ReturnValue = ParseStructList_0(Tokenizer);
    return(ReturnValue);
}
StructList ParseStructList_0(MBCC::Tokenizer& Tokenizer)
{
    StructList ReturnValue;
    while(LOOKTable[38][0][Tokenizer.Peek().Type]&& LOOKTable[38][1][Tokenizer.Peek(1).Type])
    {
        ReturnValue.Structs.push_back(ParseStructDefinition(Tokenizer));
        
    }
    return(ReturnValue);
}
