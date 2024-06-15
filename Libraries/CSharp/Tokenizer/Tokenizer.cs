using System.Collections.Generic;
using System.Text.RegularExpressions;
namespace MBCC
{

    public class TokenPosition
    {
        public int Line = 0;
        public int ByteOffset = 0;
        public TokenPosition()
        {

        }
        public TokenPosition(int NewLine,int NewByteOffset)
        {
            Line = NewLine;   
            ByteOffset = NewByteOffset;   
        }
    }

    public class Token
    {
        public int Type = 0;
        public int ByteOffset = 0;
        public TokenPosition Position = new TokenPosition();
        public string Value = "";
    }
    class TerminalRegex
    {
        public int Index = -1;
        public Regex regex;
        public TerminalRegex()
        {
               
        }

        public TerminalRegex(Regex NewRegex,int NewIndex)
        {
            regex = NewRegex;   
            Index = NewIndex;   
        }
    }
    public class Tokenizer
    {
        private List<Token> m_ReadTokens = new List<Token>();
        int m_ParseOffset = 0;
        int m_LineOffset = 0;
        int m_LineByteOffset = 0;
        int m_TokenOffset = 0;
        string m_TextData;

        Regex m_Skip;
        List<TerminalRegex> m_TerminalRegexes = new List<TerminalRegex>();

        Token p_ReadToken()
        {
            Token ReturnValue = new Token();
            ReturnValue.Type = m_TerminalRegexes.Count;
            if(m_ParseOffset == m_TextData.Length)
            {
                return(ReturnValue);
            }
            Match CurrentMatch = m_Skip.Match(m_TextData,m_ParseOffset);
            if(CurrentMatch.Length > 0)
            {
                int SkipCount = CurrentMatch.Length;
                for(int i = m_ParseOffset; i < m_ParseOffset+SkipCount;i++)
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
            if(m_ParseOffset == m_TextData.Length)
            {
                return(ReturnValue);
            }
            for(int i = 0; i < m_TerminalRegexes.Count;i++)
            {
                CurrentMatch = m_TerminalRegexes[i].regex.Match(m_TextData,m_ParseOffset);
                if(CurrentMatch.Length > 0 && CurrentMatch.Groups[m_TerminalRegexes[i].Index].Index == m_ParseOffset)
                {
                    //assert(Match.size() == 1);        
                    if(!(m_TerminalRegexes[i].Index < CurrentMatch.Groups.Count))
                    {
                        throw new System.Exception("Regex at index "+i+" have insufficient submatches");
                    }
                    ReturnValue.Value = CurrentMatch.Groups[m_TerminalRegexes[i].Index].ToString();
                    ReturnValue.Type = i;
                    ReturnValue.ByteOffset = m_ParseOffset;
                    ReturnValue.Position = new TokenPosition(m_LineOffset,m_LineByteOffset);
                    int SkipCount = CurrentMatch.Groups[0].Length;
                    for(int j = m_ParseOffset; j < m_ParseOffset+SkipCount;j++)
                    {
                        if(m_TextData[j] == '\n')
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
            if(ReturnValue.Type == m_TerminalRegexes.Count)
            {
                throw new System.Exception("Invalid character sequence: no terminal matching input at line "+ m_LineOffset +" and column " + m_LineByteOffset);
            }

            return(ReturnValue);
        }
        public void SetText(string NewText)
        {
            m_TextData = NewText;   
            m_ReadTokens = new List<Token>();
            m_ParseOffset = 0;
            m_LineOffset = 0;
            m_LineByteOffset = 0;
            m_TokenOffset = 0;
        }
        public void ConsumeToken()
        {
            m_TokenOffset += 1;
        }
        public Token Peek(int Depth = 0)
        {
            while(m_TokenOffset + Depth >= m_ReadTokens.Count)
            {
                m_ReadTokens.Add(p_ReadToken());
            }
            return m_ReadTokens[m_TokenOffset + Depth];
        }
        public Tokenizer(string SkipRegex,params string[] Regexes)
        {
            m_Skip = new Regex(SkipRegex,RegexOptions.ECMAScript);
            foreach(string NewRegex in Regexes)
            {
                if(NewRegex.Length != 0 && NewRegex[0] == '$')
                {
                    m_TerminalRegexes.Add(new TerminalRegex(new Regex(NewRegex.Substring(1),RegexOptions.ECMAScript),1));
                }   
                else
                {
                    m_TerminalRegexes.Add(new TerminalRegex(new Regex(NewRegex,RegexOptions.ECMAScript),0));
                }
            }
        }
    }
}
