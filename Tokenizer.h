#pragma once
#include <regex>
#include <deque>
#include "Token.h"
namespace MBCC
{
    class Tokenizer
    {
    private:      
        //Easy interfac, memeory map everything   
        size_t m_ParseOffset = 0;
        int m_LineOffset = 0;
        int m_LineByteOffset = 0;
        std::regex m_Skip;
        std::vector<std::pair<std::regex,int>> m_TerminalRegexes;
        std::deque<Token> m_StoredTokens;

        std::string m_TextData;

        Token p_ExtractToken();
        std::pair<int,int> p_GetLineAndPosition(size_t ParseOffset) const;  
    public: 
        //Interface
        void ConsumeToken();
        bool IsEOF(Token const& TokenToExamine);
        Token const& Peek(int Depth = 0);
        //
        void SetText(std::string NewText);
        Tokenizer(std::string const& SkipRegex,std::initializer_list<std::string> TerminalRegexes);
        std::string GetPositionString() const;
    };
}
