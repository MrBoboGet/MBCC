#include "MBCC_LSP.h"
namespace MBCC
{
        DocumentData MBCC_LSP::p_CreateDocumentData(std::string Content)
        {
            DocumentData ReturnValue;
            MBCCDefinitions Definitions;
            LSPInfo Info;
            MBLSP::LineIndex LineIndex(Content);
            try
            {
                Definitions = MBCCDefinitions::ParseDefinitions(Content.data(),Content.size(),0,Info);
            }
            catch(MBCCParseError const& e)
            {
                ReturnValue.ParseError = true;
                MBLSP::Diagnostic NewDiagnostic;
                NewDiagnostic.message  = e.ErrorMessage;
                NewDiagnostic.range.start = LineIndex.ByteOffsetToPosition(e.ParseOffset);
                NewDiagnostic.range.end = NewDiagnostic.range.start+3;
                ReturnValue.Diagnostics.push_back(NewDiagnostic);
            }
            catch(std::exception const& e)
            {
                ReturnValue.ParseError = true;
            }
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
                else if(Token.Type == DefinitionsTokenType::AssignedLHS)
                {
                    NewToken.Type = MBLSP::TokenType::Property;
                }
                else if(Token.Type == DefinitionsTokenType::AssignedRHS)
                {
                    NewToken.Type = MBLSP::TokenType::Property;
                }
                ReturnValue.SemanticTokens.push_back(NewToken);
            }
            for(auto const& Diagnostic : Info.Diagnostics)
            {
                MBLSP::Diagnostic NewDiagnostic;
                NewDiagnostic.message  = Diagnostic.Message;
                NewDiagnostic.range.start = LineIndex.ByteOffsetToPosition(Diagnostic.ByteOffset);
                NewDiagnostic.range.end = NewDiagnostic.range.start+Diagnostic.Length;
                ReturnValue.Diagnostics.push_back(NewDiagnostic);
            }
            ReturnValue.Content = std::move(Content);
            ReturnValue.RawInfo = std::move(Info);
            ReturnValue.Index = std::move(LineIndex);
            ReturnValue.Definitions = std::move(Definitions);
            return(ReturnValue);
        }
        void MBCC_LSP::p_PushDiagnostics(DocumentData const& DocumentData,std::string const& URI)
        {
            MBLSP::PublishDiagnostics_Notification Notification;
            Notification.params.diagnostics = DocumentData.Diagnostics;
            Notification.params.uri = URI;
            m_AssociatedHandler->SendNotification(Notification);
        }
        MBLSP::Initialize_Response MBCC_LSP::HandleRequest(MBLSP::InitializeRequest const& Request)
        {
            MBLSP::Initialize_Response ReturnValue;
            ReturnValue.result = MBLSP::Initialize_Result();
            ReturnValue.result->capabilities.textDocumentSync = MBLSP::TextDocumentSyncOptions();
            ReturnValue.result->capabilities.definitionProvider = true;
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
            p_PushDiagnostics(m_OpenedDocuments[URI],URI);
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
            p_PushDiagnostics(m_OpenedDocuments[URI],URI);
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
            p_PushDiagnostics(m_OpenedDocuments[URI],URI);
        }

        void MBCC_LSP::SetHandler(MBLSP::LSP_ServerHandler* AssociatedHandler)
        {
            m_AssociatedHandler = AssociatedHandler;
        }
        std::string h_ExtractString(DocumentData const& Info,DefinitionsToken Token)
        {
            return(std::string(Info.Content.begin()+Token.ByteOffset,
                Info.Content.begin()+Token.ByteOffset+Token.Length));
        }
        size_t h_GetNonTerminalMember(MBCCDefinitions const& Defs,  std::string const& NonTerminalName,std::string const& MemberName)
        {
            size_t ReturnValue = -1;
            if(auto NonTermIt = Defs.NameToNonTerminal.find(NonTerminalName); 
                    NonTermIt != Defs.NameToNonTerminal.end())
            {
                if(Defs.NonTerminals[NonTermIt->second].AssociatedStruct != -1)
                {
                    auto const& Struct = Defs.Structs[Defs.NonTerminals[NonTermIt->second].AssociatedStruct];
                    if(MemberName == "this")
                    {
                        ReturnValue = Struct.StructBegin;
                    }
                    else
                    {
                        for(auto const& Member : Struct.MemberVariables)
                        {
                            if(Member.GetName() == MemberName)
                            {
                                size_t MemberBegin = 0;
                                Member.Visit([&](MemberVariable const& Value)
                                        {
                                        MemberBegin = Value.BeginOffset;
                                        });
                                ReturnValue = MemberBegin;
                                break;
                            }   
                        }
                    }
                }
            }
            return(ReturnValue);
        }
        MBLSP::GotoDefinition_Response MBCC_LSP::HandleRequest(MBLSP::GotoDefinition_Request const& Request)
        {
            MBLSP::GotoDefinition_Response ReturnValue;
            ReturnValue.result = std::vector<MBLSP::Location>();
            auto DocumentIt = m_OpenedDocuments.find(Request.params.textDocument.uri);
            if(DocumentIt != m_OpenedDocuments.end())
            {
                size_t LineByteOffset = DocumentIt->second.Index.PositionToByteOffset(Request.params.position);
                auto TokenIt = std::lower_bound(DocumentIt->second.RawInfo.SemanticsTokens.begin(),
                    DocumentIt->second.RawInfo.SemanticsTokens.end(),LineByteOffset,[](DefinitionsToken const& lhs,size_t rhs)
                    {
                        return(lhs.ByteOffset + lhs.Length < rhs);
                    });
                if(TokenIt != DocumentIt->second.RawInfo.SemanticsTokens.end())
                {
                    if(TokenIt->ByteOffset <= LineByteOffset && LineByteOffset <= TokenIt->ByteOffset+TokenIt->Length)
                    {
                        auto const& StoredLSPInfo = DocumentIt->second.RawInfo;
                        std::string TokenValue = std::string(DocumentIt->second.Content.begin()+TokenIt->ByteOffset,
                            DocumentIt->second.Content.begin()+TokenIt->ByteOffset+TokenIt->Length);
                        MBLSP::Location NewLocation;
                        NewLocation.uri = Request.params.textDocument.uri;
                        if(TokenIt->Type == DefinitionsTokenType::NonTerminal)
                        {
                            if(auto DefIt = StoredLSPInfo.NonTerminalDefinitions.find(TokenValue); DefIt != StoredLSPInfo.NonTerminalDefinitions.end())
                            {
                                NewLocation.range.start = DocumentIt->second.Index.ByteOffsetToPosition(DefIt->second);
                                NewLocation.range.end = NewLocation.range.start+1;
                                ReturnValue.result->push_back(NewLocation);
                            }
                        }
                        else if(TokenIt->Type == DefinitionsTokenType::Terminal)
                        {
                            if(auto DefIt = StoredLSPInfo.TerminalDefinitions.find(TokenValue); DefIt != StoredLSPInfo.TerminalDefinitions.end())
                            {
                                NewLocation.range.start = DocumentIt->second.Index.ByteOffsetToPosition(DefIt->second);
                                NewLocation.range.end = NewLocation.range.start+1;
                                ReturnValue.result->push_back(NewLocation);
                            }
                        }
                        else if(TokenIt->Type == DefinitionsTokenType::Class)
                        {
                            if(auto DefIt = StoredLSPInfo.StructureDefinitions.find(TokenValue); DefIt != StoredLSPInfo.StructureDefinitions.end())
                            {
                                NewLocation.range.start = DocumentIt->second.Index.ByteOffsetToPosition(DefIt->second);
                                NewLocation.range.end = NewLocation.range.start+1;
                                ReturnValue.result->push_back(NewLocation);
                            }
                        }
                        else if(TokenIt->Type == DefinitionsTokenType::Rule)
                        {
                            if(auto DefIt = StoredLSPInfo.NonTerminalDefinitions.find(TokenValue); DefIt != StoredLSPInfo.NonTerminalDefinitions.end())
                            {
                                NewLocation.range.start = DocumentIt->second.Index.ByteOffsetToPosition(DefIt->second);
                                NewLocation.range.end = NewLocation.range.start+1;
                                ReturnValue.result->push_back(NewLocation);
                            }
                            else if(auto DefIt = StoredLSPInfo.TerminalDefinitions.find(TokenValue); DefIt != StoredLSPInfo.TerminalDefinitions.end())
                            {
                                NewLocation.range.start = DocumentIt->second.Index.ByteOffsetToPosition(DefIt->second);
                                NewLocation.range.end = NewLocation.range.start+1;
                                ReturnValue.result->push_back(NewLocation);
                            }
                        }
                        else if(TokenIt->Type == DefinitionsTokenType::AssignedRHS)
                        {
                            size_t CurrentTokenOffset = TokenIt-StoredLSPInfo.SemanticsTokens.begin();
                            while(CurrentTokenOffset != -1)
                            {
                                if(StoredLSPInfo.SemanticsTokens[CurrentTokenOffset].Type  == DefinitionsTokenType::Rule)
                                {
                                    break;
                                }   
                                CurrentTokenOffset--;
                            }
                            auto const& Definitions = DocumentIt->second.Definitions;
                            if(CurrentTokenOffset != -1)
                            {
                                size_t Position = h_GetNonTerminalMember(Definitions,
                                        h_ExtractString(DocumentIt->second,StoredLSPInfo.SemanticsTokens[CurrentTokenOffset]),TokenValue);
                                if(Position != -1)
                                {
                                    NewLocation.range.start = DocumentIt->second.Index.ByteOffsetToPosition(Position);
                                    NewLocation.range.end = NewLocation.range.start+1;
                                    ReturnValue.result->push_back(NewLocation);
                                }
                            }
                        }
                        else if(TokenIt->Type == DefinitionsTokenType::AssignedLHS)
                        {
                            size_t CurrentTokenOffset = TokenIt-StoredLSPInfo.SemanticsTokens.begin();
                            while(CurrentTokenOffset != -1)
                            {
                                if(StoredLSPInfo.SemanticsTokens[CurrentTokenOffset].Type  == DefinitionsTokenType::NonTerminal)
                                {
                                    break;
                                }   
                                CurrentTokenOffset--;
                            }
                            auto const& Definitions = DocumentIt->second.Definitions;
                            if(CurrentTokenOffset != -1)
                            {
                                size_t Position = h_GetNonTerminalMember(Definitions,
                                        h_ExtractString(DocumentIt->second,StoredLSPInfo.SemanticsTokens[CurrentTokenOffset]),TokenValue);
                                if(Position != -1)
                                {
                                    NewLocation.range.start = DocumentIt->second.Index.ByteOffsetToPosition(Position);
                                    NewLocation.range.end = NewLocation.range.start+1;
                                    ReturnValue.result->push_back(NewLocation);
                                }
                            }
                        }
                    }
                }
            }
            return(ReturnValue);
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
