#include "MBCC_LSP.h"
namespace MBCC
{
        DocumentData MBCC_LSP::p_CreateDocumentData(std::string const& Content)
        {
            DocumentData ReturnValue;
            MBCCDefinitions Definitions;
            LSPInfo Info;
            try
            {
                Definitions = MBCCDefinitions::ParseDefinitions(Content.data(),Content.size(),0,Info);
            }
            catch(MBCCParseError const& e)
            {
                ReturnValue.ParseError = true;
            }
            catch(std::exception const& e)
            {
                   
            }
            MBLSP::LineIndex LineIndex(Content);
            for(auto const& Token : Info.SemanticsTokens)
            {
                MBLSP::SemanticToken NewToken;
                NewToken.Pos = LineIndex.ByteOffsetToPosition(Token.ByteOffset);
                NewToken.Length = Token.Length;
                //Null,
                //Terminal,
                //NonTerminal,
                //Class,
                //Struct,
                //Keyword,
                //Variable,
                //SpecialToken,
                //String
                if(Token.Type == DefinitionsTokenType::String)
                {
                    NewToken.Type = MBLSP::TokenType::String;   
                }
                else if(Token.Type == DefinitionsTokenType::Keyword)
                {
                    NewToken.Type = MBLSP::TokenType::Keyword;   
                }
                else if(Token.Type == DefinitionsTokenType::Class)
                {
                    NewToken.Type = MBLSP::TokenType::Class;   
                }
                else if(Token.Type == DefinitionsTokenType::NonTerminal)
                {
                    NewToken.Type = MBLSP::TokenType::Macro;
                }
                else if(Token.Type == DefinitionsTokenType::Terminal)
                {
                    NewToken.Type = MBLSP::TokenType::Boolean;
                }
                else if(Token.Type == DefinitionsTokenType::Rule)
                {
                    NewToken.Type = MBLSP::TokenType::Macro;
                    if(Definitions.NameToTerminal.find(std::string(Content.begin()+Token.ByteOffset,Content.begin()+Token.ByteOffset+Token.Length))
                            != Definitions.NameToTerminal.end())
                    {
                        NewToken.Type = MBLSP::TokenType::Boolean;
                    }
                }
                else if(Token.Type == DefinitionsTokenType::Variable)
                {
                    NewToken.Type = MBLSP::TokenType::Property;
                }
                ReturnValue.SemanticTokens.push_back(NewToken);
            }
            return(ReturnValue);
        }
        void MBCC_LSP::p_PushDiagnostics(DocumentData const& DocumentData,std::string const& URI)
        {
            MBLSP::PublishDiagnostics_Notification Notification;
            Notification.params.diagnostics = DocumentData.Diagnostics;
            m_AssociatedHandler->SendNotification(Notification);
        }
        MBLSP::Initialize_Response MBCC_LSP::HandleRequest(MBLSP::InitializeRequest const& Request)
        {
            MBLSP::Initialize_Response ReturnValue;
            ReturnValue.result = MBLSP::Initialize_Result();
            ReturnValue.result->capabilities.textDocumentSync = MBLSP::TextDocumentSyncOptions();
            //ReturnValue.result->capabilities.declarationProvider = true;
            ReturnValue.result->capabilities.semanticTokensProvider = MBLSP::SemanticTokensOptions();
            MBLSP::SemanticTokensLegend Legend;
            Legend.tokenTypes = MBLSP::GetTokenLegend();
            ReturnValue.result->capabilities.semanticTokensProvider->legend = Legend;
            ReturnValue.result->capabilities.semanticTokensProvider->full = true;
            ReturnValue.result->capabilities.semanticTokensProvider->range = true;
            return(ReturnValue); 
        }
        void MBCC_LSP::OpenedDocument(std::string const& URI,std::string const& Content)
        {
            m_OpenedDocuments[URI] = p_CreateDocumentData(Content);
        }
        void MBCC_LSP::ClosedDocument(std::string const& URI)
        {
            auto DocumentIt = m_OpenedDocuments.find(URI);
            if(DocumentIt != m_OpenedDocuments.end())
            {
                m_OpenedDocuments.erase(DocumentIt);   
            }
        }
        void MBCC_LSP::DocumentChanged(std::string const& URI,std::string const& NewContent)
        {
            m_OpenedDocuments[URI] = p_CreateDocumentData(NewContent);
        }
        void MBCC_LSP::DocumentChanged(std::string const& URI,std::string const& NewContent,std::vector<MBLSP::TextChange> const& Changes)
        {
            DocumentData NewData =  p_CreateDocumentData(NewContent);
            if(auto DocumentIt = m_OpenedDocuments.find(URI); DocumentIt != m_OpenedDocuments.end())
            {
                if(NewData.ParseError)
                {
                    std::vector<MBLSP::SemanticToken> UpdatedOldTokens = MBLSP::UpdateSemanticTokens(DocumentIt->second.SemanticTokens,Changes);
                    NewData.SemanticTokens = MBLSP::CombineTokens(NewData.SemanticTokens,UpdatedOldTokens);
                    //NewData.SemanticTokens = MBLSP::UpdateSemanticTokens(DocumentIt->second.SemanticTokens,Changes);
                }
            }
            m_OpenedDocuments[URI] = NewData;
        }

        void MBCC_LSP::SetHandler(MBLSP::LSP_ServerHandler* AssociatedHandler)
        {
            m_AssociatedHandler = AssociatedHandler;
        }
        MBLSP::SemanticToken_Response MBCC_LSP::HandleRequest(MBLSP::SemanticToken_Request const& Request)
        {
            MBLSP::SemanticToken_Response Result;
            auto DocumentIt = m_OpenedDocuments.find(Request.params.textDocument.uri);
            if(DocumentIt  != m_OpenedDocuments.end())
            {
                Result.result = MBLSP::SemanticTokens();
                Result.result->data =  MBLSP::CalculateSemanticTokens(DocumentIt->second.SemanticTokens);
            }
            return(Result);
        }
        MBLSP::SemanticTokensRange_Response MBCC_LSP::HandleRequest(MBLSP::SemanticTokensRange_Request const& Request)
        {
            MBLSP::SemanticTokensRange_Response Result;
            auto DocumentIt = m_OpenedDocuments.find(Request.params.textDocument.uri);
            if(DocumentIt  != m_OpenedDocuments.end())
            {
                Result.result = MBLSP::SemanticTokens();
                Result.result->data =  MBLSP::GetTokenRange(MBLSP::CalculateSemanticTokens(DocumentIt->second.SemanticTokens),Request.params.range);
            }
            return(Result);
        }
}
