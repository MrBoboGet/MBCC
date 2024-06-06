#include "Tokenizer.h"
#include <assert.h>
namespace MBCC
{
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
        for(int i = 0; i < m_TerminalRegexes.size();i++)
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


    //
}
