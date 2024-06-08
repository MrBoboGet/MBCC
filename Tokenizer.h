#pragma once
#include <regex>
#include <deque>
#include "Token.h"
namespace MBCC
{
    template<typename T>
    class TokenizerBase
    {
    protected:
        std::deque<Token> m_StoredTokens;
        int m_LineOffset = 0;
        int m_LineByteOffset = 0;
    private:      
        //Easy interfac, memeory map everything   
        size_t m_ParseOffset = 0;
        std::regex m_Skip;
        std::vector<std::pair<std::regex,int>> m_TerminalRegexes;


        //std::string m_TextData;

        T front = T();
        T end = T();

        Token p_ExtractToken() 
        {
            Token ReturnValue;
            ReturnValue.Type = m_TerminalRegexes.size()+1;
            if(front == end)
            {
                return(ReturnValue);
            }
            std::match_results<T> Match;
            //std::string const& TextRef = m_TextData;
            if(std::regex_search(front,end,Match,m_Skip,std::regex_constants::match_continuous))
            {
                assert(Match.size() == 1);
                size_t SkipCount = Match[0].length();
                for(size_t i = 0; i < SkipCount;i++)
                {
                    if(*front == '\n')
                    {
                        m_LineOffset += 1;
                        m_LineByteOffset = 0;
                    }   
                    else
                    {
                        m_LineByteOffset += 1;   
                    }
                    ++front;
                }
                m_ParseOffset += SkipCount;
            }
            if(front == end)
            {
                return(ReturnValue);
            }
            for(int i = 0; i < m_TerminalRegexes.size();i++)
            {
                if(std::regex_search(front,end,Match,m_TerminalRegexes[i].first,std::regex_constants::match_continuous))
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
                    for(size_t i = 0; i < SkipCount;i++)
                    {
                        if(*front == '\n')
                        {
                            m_LineOffset += 1;
                            m_LineByteOffset = 0;
                        }   
                        else
                        {
                            m_LineByteOffset += 1;   
                        }
                        ++front;
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
        std::pair<int,int> p_GetLineAndPosition(size_t TargetPosition) const
        {
            std::pair<int,int> ReturnValue = {1,1};
            ReturnValue.first += m_LineOffset;
            ReturnValue.second += m_LineByteOffset;
            return ReturnValue;
        }
    public: 
        //Interface
        void ConsumeToken()
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
        T& GetFront()
        {
            return front;
        }
        bool IsEOF(Token const& TokenToExamine)
        {
            return(TokenToExamine.Type == m_TerminalRegexes.size()+1);
        }
        Token const& Peek(int Depth = 0)
        {
            while(m_StoredTokens.size() <= Depth)
            {
                m_StoredTokens.push_back(p_ExtractToken()); 
            }
            return(m_StoredTokens[Depth]);
        }
        TokenizerBase(std::string const& SkipRegex,std::initializer_list<std::string> TerminalRegexes)
        {
            SetRegexes(SkipRegex,TerminalRegexes);
        }
        void SetRegexes(std::string const& SkipRegex,std::vector<std::string> const& TerminalRegexes)
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
        TokenizerBase()
        {
               
        }
        void SetData(T front,T end)
        {
            m_ParseOffset = 0;       
            m_LineByteOffset = 0;
            m_LineOffset = 0;
            //m_TextData = std::move(NewText);
            this->front = front;
            this->front = front;
            m_StoredTokens.clear();
        }
        std::string GetPositionString() const
        {
            std::string ReturnValue;       
            std::pair<int,int> LineAndPosition = {m_LineOffset+1,m_LineByteOffset+1};
            if(m_StoredTokens.size() > 0)
            {
                LineAndPosition.first = m_StoredTokens.front().Position.Line+1;
                LineAndPosition.second = m_StoredTokens.front().Position.ByteOffset+1;
            }
            ReturnValue = "line "+std::to_string(LineAndPosition.first)+", col "+std::to_string(LineAndPosition.second);
            return(ReturnValue);
        }
        //
    };

    class Tokenizer : public TokenizerBase<const char *>
    {
        std::string m_Text;
    public:
        void SetText(std::string NewText)
        {
            m_Text = std::move(NewText);
            SetData(m_Text.data(),m_Text.data()+m_Text.size());
        }
        Tokenizer(std::string const& SkipRegex,std::initializer_list<std::string> TerminalRegexes)
            : TokenizerBase<const char*>(SkipRegex,TerminalRegexes)
        {
               
        }
    };
}
