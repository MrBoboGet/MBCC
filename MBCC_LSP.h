#include "MBCC.h"



namespace MBCC
{
    struct DocumentData
    {
        bool ParseError = false;
        MBCCDefinitions Definitions;
        LSPInfo RawInfo;
        std::string Content;
        MBLSP::LineIndex Index;
        std::vector<MBLSP::Diagnostic> Diagnostics;
        std::vector<MBLSP::SemanticToken> SemanticTokens;
    };
    class MBCC_LSP : public MBLSP::LSP_Server
    {
    private:
        MBLSP::LSP_ServerHandler* m_AssociatedHandler = nullptr;

        std::unordered_map<std::string,DocumentData> m_OpenedDocuments;
        DocumentData p_CreateDocumentData(std::string Content);

        void p_PushDiagnostics(DocumentData const& DocumentData,std::string const& URI);
    public:      
        virtual MBLSP::Initialize_Response HandleRequest(MBLSP::InitializeRequest const& Request) override;
        virtual void OpenedDocument(std::string const& URI,std::string const& Content) override;
        virtual void ClosedDocument(std::string const& URI) override;
        virtual void DocumentChanged(std::string const& URI,std::string const& NewContent) override;
        virtual void DocumentChanged(std::string const& URI,std::string const& NewContent,std::vector<MBLSP::TextChange> const& Changes) override;
        virtual void SetHandler(MBLSP::LSP_ServerHandler* AssociatedHandler) override;

        MBLSP::GotoDefinition_Response HandleRequest(MBLSP::GotoDefinition_Request const& Request) override;
        MBLSP::SemanticToken_Response HandleRequest(MBLSP::SemanticToken_Request const& Request) override;
        MBLSP::SemanticTokensRange_Response HandleRequest(MBLSP::SemanticTokensRange_Request const& Request) override;
    };
};
