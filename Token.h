#pragma once
#include <string>
namespace MBCC
{
       
    struct TokenPosition
    {
        int Line = 0;    
        int ByteOffset = 0;
        TokenPosition() {}
        TokenPosition(int LinePos,int NewByteOffset)
        {
            Line = LinePos;
            ByteOffset = NewByteOffset;   
        }
        TokenPosition operator+(int Rhs) const
        {
            TokenPosition ReturnValue = *this;   
            ReturnValue.ByteOffset += Rhs;
            return ReturnValue;
        }
        bool operator<(TokenPosition const& Rhs) const noexcept
        {
            bool ReturnValue = false;
            if(Line < Rhs.Line)
            {
                ReturnValue = true;    
            } else if(Line == Rhs.Line)
            {
                ReturnValue = ByteOffset < Rhs.ByteOffset;
            }
            return(ReturnValue);
        }
    };
    struct Token
    {
        //TerminalIndex Type = -1;
        int Type = -1;
        size_t ByteOffset = 0;
        TokenPosition Position;
        std::string Value;    
    };
}
