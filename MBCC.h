#pragma once
#include <stdint.h>
#include <string>
#include <type_traits>
#include <vector>
#include <unordered_map>
#include <MBUtility/MBErrorHandling.h>
#include <MBUtility/MBInterfaces.h>
#include <utility>
#include <variant>
#include <regex>
#include <deque>
#include <MBUtility/MBMatrix.h>
#include <set>




#include <MBLSP/MBLSP.h>
#include <MBLSP/SemanticTokens.h>

#include <assert.h>
namespace MBCC
{

    struct Identifier
    {
        std::string Value;
        size_t ByteOffset = 0;
    };
    template<typename T>
    int GetTypeBegin()
    {

        static_assert(std::is_same<T,T>::value && false,"GetTypeBegin only works with template specialization");  
        return(-1);
    } 
    template<typename T>
    int GetTypeEnd()
    {
        static_assert(std::is_same<T,T>::value && false,"GetTypeEnd only works with template specialization");  
        return(-1);
    } 
    class AST_Base
    {
    public:
        virtual std::unique_ptr<AST_Base> Copy() const = 0; 
        virtual ~AST_Base()
        {
               
        }
    };
    template<typename T>
    std::unique_ptr<AST_Base> CopyAST(T const& ObjectToCopy)
    {
        return(std::unique_ptr<AST_Base>(new T(ObjectToCopy)));
    }
    //class AST_Copier : public AST_Base
    //{
    //public:
    //    virtual std::unique_ptr<AST_Base> Copy() const override
    //    {
    //        return(std::unique_ptr<AST_Base>(new T(static_cast<T const&>(*this))));
    //    }
    //    typedef T This;
    //};
    template<typename C>
    class PolyBase
    {
    protected:
        std::unique_ptr<C> m_Data;
        int m_TypeID = -1;
        public:
        //template<typename T> void Accept(T& Visitor);
        //template<typename T> void Accept(T& Visitor) const;
        template<typename T,typename = std::enable_if_t<std::is_base_of<C,T>::value>> PolyBase(T ObjectToStore)
        {
            static_assert(std::is_base_of<C,T>::value);
            m_Data = std::unique_ptr<C>(new T(std::move(ObjectToStore)));
            m_TypeID = GetTypeBegin<T>();
        }
        PolyBase(std::unique_ptr<C> NewData,int Type)
        {
            m_Data = std::move(NewData);
            m_TypeID = Type;
        }
        PolyBase(PolyBase const& BaseToCopy)
        {
            m_TypeID = BaseToCopy.m_TypeID;
            if(BaseToCopy.m_Data != nullptr)
            {
                std::unique_ptr<AST_Base> CopiedStruct = BaseToCopy.m_Data->Copy();
                AST_Base* CopiedPointer = CopiedStruct.release();
                m_Data = std::unique_ptr<C>(static_cast<C*>(CopiedPointer));
            }
        }
        PolyBase() = default;
        PolyBase(PolyBase&&) = default;
        template<typename T> bool IsType() const
        {
            static_assert(std::is_base_of_v<C,T>, "Can only check type for possible derived value");
            auto IDToCompare = GetTypeBegin<T>();
            return(IDToCompare <= m_TypeID && m_TypeID < GetTypeEnd<T>());
        }
        bool IsEmpty() const
        {
            return m_Data == nullptr;
        }
        void operator=(PolyBase StructToMove)
        {
            std::swap(m_TypeID,StructToMove.m_TypeID);
            std::swap(m_Data,StructToMove.m_Data);
        }
        template<typename T> T const& GetType() const
        {
            if(!IsType<T>() || m_Data == nullptr)
            {
                throw std::runtime_error("Invalid type access for abstract AST class");
            }
            return static_cast<T const&>(*m_Data);
        }
        template<typename T> T& GetType()
        {
            if(!IsType<T>() || m_Data == nullptr)
            {
                throw std::runtime_error("Invalid type access for abstract AST class");
            }
            return static_cast<T&>(*m_Data);
        }
        C& GetBase()
        {
            if(m_Data == nullptr)
            {
                throw std::runtime_error("Invalid type access for abstract AST class: data is null");
            }
            return static_cast<C&>(*m_Data);
        }
        C const& GetBase() const
        {
            if(m_Data == nullptr)
            {
                throw std::runtime_error("Invalid type access for abstract AST class: data is null");
            }
            return static_cast<C const&>(*m_Data);
        }
    };
    class MemberVariable
    {
    public:
        std::string Name; 
        Identifier DefaultValue;
        size_t BeginOffset = 0;
    };
    class StructMemberVariable_List : public MemberVariable
    {
    public:
        std::string ListType;
        size_t ListByteOffset = 0;
    };
    class StructMemberVariable_Raw : public MemberVariable
    {
    public:
        std::string RawMemberType;
    };
    class StructMemberVariable_Struct : public MemberVariable
    {
    public:
        std::string StructType;
    };
    class StructMemberVariable_Int : public MemberVariable
    {
    public:
        int Value = 0;
    };
    class StructMemberVariable_tokenPosition : public MemberVariable
    {
    public:
    };
    class StructMemberVariable_Bool : public MemberVariable
    {
    public:
        bool Value = false;
    };
    class StructMemberVariable_String : public MemberVariable
    {
    public:
        std::string Value;
    };
    //The more I use this class the more I realise that using the variant directly is most likely more than
    //enough
    class MemberVariableVisitor;
    class StructMemberVariable
    {
        std::variant<StructMemberVariable_Raw,StructMemberVariable_List,StructMemberVariable_Struct,StructMemberVariable_Int,StructMemberVariable_String,
            StructMemberVariable_Bool,StructMemberVariable_tokenPosition>
            m_Content;
    public:
        StructMemberVariable() = default;
        StructMemberVariable(StructMemberVariable_List ListMemberVariable);
        StructMemberVariable(StructMemberVariable_Raw RawMemberVariable);
        StructMemberVariable(StructMemberVariable_Struct StructMemberVariable);
        StructMemberVariable(StructMemberVariable_Int RawMemberVariable);
        StructMemberVariable(StructMemberVariable_Bool RawMemberVariable);
        StructMemberVariable(StructMemberVariable_String StructMemberVariable);
        StructMemberVariable(StructMemberVariable_tokenPosition StructMemberVariable);
        std::string& GetName();
        Identifier& GetDefaultValue();
        std::string const& GetName() const;
        Identifier const& GetDefaultValue() const;

        MemberVariable& GetBase()
        {
            MemberVariable* Result = nullptr;    
            std::visit([&](MemberVariable& var)
                    {
                        Result = &var;
                    },m_Content);
            assert(Result != nullptr && "std::variant should always be initialised with base of type MemberVariable");
            return(*Result);
        }
        MemberVariable const& GetBase() const
        {
            MemberVariable const* Result = nullptr;    
            std::visit([&](MemberVariable const& var)
                    {
                        Result = &var;
                    },m_Content);
            assert(Result != nullptr && "std::variant should always be initialised with base of type MemberVariable");
            return(*Result);
        }
        template<typename T>
        bool IsType() const
        {
            return(std::holds_alternative<T>(m_Content));    
        }
        template<typename T>
        T& GetType()
        {
            return(std::get<T>(m_Content)); 
        }
        template<typename T>
        T const& GetType() const
        {
            return(std::get<T>(m_Content)); 
        }
        template<typename T>
        void Visit(T Visitor)
        {
            std::visit(Visitor,m_Content);
        }
        template<typename T>
        void Visit(T Visitor) const
        {
            std::visit(Visitor,m_Content);
        }

        void Accept(MemberVariableVisitor& Visitor);
    };
    class MemberVariableVisitor
    {
    public:
        void Visit(StructMemberVariable_Raw const& Raw){};
        void Visit(StructMemberVariable_List const& List){};
        void Visit(StructMemberVariable_Struct const& Struct){};
        void Visit(StructMemberVariable_Int const& Int){};
        void Visit(StructMemberVariable_Bool const& Int){};
        void Visit(StructMemberVariable_String const& String){};
    };
    struct StructDefinition
    {
        size_t StructBegin = 0;
        std::string Name; 
        std::string ParentStruct;
        size_t ParentOffset = 0;
        std::vector<StructMemberVariable> MemberVariables;
        //bool HasMember(std::string const& MemberToCheck) const;
        //StructMemberVariable const& GetMember(std::string const& MemberName) const;
        //StructMemberVariable& GetMember(std::string const& MemberName);
    };
    typedef int RuleIndex;
    typedef int TerminalIndex;
    typedef int NonTerminalIndex;
    typedef int ParseIndex;
    typedef uint_least32_t StructIndex;
    typedef uint_least32_t TypeInfo;

    namespace TypeFlags
    {
        constexpr uint_least32_t List = 1u<<31;   
        constexpr uint_least32_t String = 1u<<30;   
        constexpr uint_least32_t Int = 1u<<29;   
        constexpr uint_least32_t Bool = 1u<<28;   
        constexpr uint_least32_t Token = 1u<<27;  
        constexpr uint_least32_t TokenPos = 1u<<26;
        constexpr uint_least32_t Raw = 1u<<25;   

        constexpr unsigned int Base = ~(List|String|Int|Bool|Token|Raw|TokenPos);
    }
    struct MemberReference
    {
        std::vector<TypeInfo> PartTypes;
        std::vector<std::string> Names;
        std::vector<size_t> PartByteOffsets;
    };
    struct Literal
    {
        std::string LiteralString;
    };
    class MemberExpression
    {
        struct Empty{};
        std::variant<Empty,MemberReference,Literal> m_Data;
    public:
        TypeInfo ResultType = -1;

        bool IsEmpty() const
        {
            return IsType<Empty>();
        }
        template<typename T>
        bool IsType() const
        {
            return std::holds_alternative<T>(m_Data);   
        }
        template<typename T>
        T& SetType()
        {
            m_Data = T();
            return std::get<T>(m_Data);
        }
        template<typename T>
        T& GetType()
        {
            return std::get<T>(m_Data);
        }
        template<typename T>
        T const& GetType() const
        {
            return std::get<T>(m_Data);
        }
    };
    struct RuleComponent
    {
        bool IsTerminal = false;
        bool IsInline = false;
        int Min = 1;
        //-1 means that Max is unbounded
        int Max = 1;
        MemberExpression AssignedMember;
        int AssignOrder = -1;//Indicates no special order
        MemberExpression ReferencedRule;
        //can either be a RuleIndex, or a TerminalIndex depending
        ParseIndex ComponentIndex = 0;
    };
    //TODO fix so that semantic actions doesn't just just use textual insertion
    struct SemanticAction
    {
        std::string ActionString;      
    };
    struct ParseRule
    {
        bool NeedsAssignmentOrder = false;
        std::vector<RuleComponent> Components;
        std::vector<SemanticAction> Actions;
        std::vector<std::pair<int,RuleComponent>> MetaComponents;
    };
    struct Terminal
    {
        std::string Name;        
        std::string RegexDefinition;        
    };
    struct NonTerminal
    {
        std::string Name;    
        bool IsInline = false;
        StructIndex AssociatedStruct = -1;
        std::vector<ParseRule> Rules;
    };
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
        TerminalIndex Type = -1;
        size_t ByteOffset = 0;
        TokenPosition Position;
        std::string Value;    
    };
    struct DependancyInfo
    {
        std::vector<StructIndex> StructureDependancyOrder; 
        //Would it be faster to use a matrix?
        std::vector<std::set<StructIndex>> ChildrenMap;

        bool HasChildren(StructIndex Struct) const
        {
            return ChildrenMap[Struct].size() > 0;   
        }
    };
    

    enum class DefinitionsTokenType
    {
        Null,
        Terminal,
        NonTerminal,
        Class,
        Rule,
        Keyword,
        Variable,
        AssignedRHS,
        AssignedLHS,
        String,
        Bool,
        Number,
    };
    struct DefinitionsToken
    {
        DefinitionsToken()
        {
               
        }
        DefinitionsToken(size_t NewByteOffset,int NewLength,DefinitionsTokenType NewType)
        {
            ByteOffset = NewByteOffset;
            Length = NewLength;
            Type = NewType;
        }
        DefinitionsToken(Identifier const& IdentifierToConvert,DefinitionsTokenType NewType)
        {
            ByteOffset = IdentifierToConvert.ByteOffset;
            Length = IdentifierToConvert.Value.size();
            Type = NewType;
        }
        bool operator<(DefinitionsToken const& rhs) const
        {
            return(ByteOffset < rhs.ByteOffset);
        }
        size_t ByteOffset = 0;
        int Length = 0;
        DefinitionsTokenType Type = DefinitionsTokenType::Null;
    };
    struct Diagnostic
    {
        size_t ByteOffset = 0;   
        size_t Length = 0;
        std::string Message;
        Diagnostic() = default;
        Diagnostic(size_t NewByteOffset,size_t NewLength,std::string NewMessage)
        {
            ByteOffset = NewByteOffset;   
            Length = NewLength;
            Message = std::move(NewMessage);
        }
        Diagnostic(Identifier AssociatedIdentifier,std::string NewMessage)
        {
            ByteOffset =  AssociatedIdentifier.ByteOffset;
            Length = AssociatedIdentifier.Value.size();
            Message = std::move(NewMessage);
        }
    };
    struct LSPInfo
    {
        std::vector<Diagnostic> Diagnostics;
        std::vector<DefinitionsToken> SemanticsTokens;
        std::unordered_map<std::string,size_t> StructureDefinitions;
        std::unordered_map<std::string,size_t> TerminalDefinitions;
        std::unordered_map<std::string,size_t> NonTerminalDefinitions;
        //begins?
    };
    class MBCCParseError : public std::exception
    {
    public:
        std::string ErrorMessage;
        size_t ParseOffset = 0;
        MBCCParseError(std::string Error,size_t NewParseOffset)
        {
            ErrorMessage = Error;
            ParseOffset = NewParseOffset;
        }
        const char* what() const noexcept override 
        {
            return(ErrorMessage.data());
        }
    };
    class MBCCDefinitions
    {
    private:
        //might need to introduce alias for rules, otherwise
        //error in parsing lambdas result in completelty unreadable results
        struct Lambda
        {
            Identifier Type;
            std::string Name;
            std::vector<ParseRule> Rules;
        };
        static std::string p_LambdaIDToLambdaName(int ID);

        void p_VerifyStructs(LSPInfo& OutInfo);
        //MEGA ugly, refactor
        void p_VerifyComponent(RuleComponent& ComponentToVerify,std::string const& NonTerminalName,StructDefinition const* AssociatedStruct,bool& ThisAssignment,bool& RegularAssignment,LSPInfo& OutInfo);
        void p_VerifyRules(LSPInfo& OutInfo);
        void p_UpdateReferencesAndVerify(LSPInfo& OutInfo);


        
        static Identifier p_ParseIdentifier(const char* Data,size_t DataSize,size_t ParseOffset,size_t* OutParseOffset);
        static SemanticAction p_ParseSemanticAction(const char* Data,size_t DataSize,size_t ParseOffset,size_t* OutParseOffset);
        static Terminal p_ParseTerminal(const char* Data,size_t DataSize,size_t ParseOffset,size_t* OutParseOffset,LSPInfo& OutInfo);
        static std::pair<Identifier,Identifier> p_ParseDef(const char* Data,size_t DataSize,size_t ParseOffset,size_t* OutParseOffset,LSPInfo& OutInfo);
        static StructMemberVariable p_ParseMemberVariable(const char* Data,size_t DataSize,size_t ParseOffset,size_t* OutParseOffset,LSPInfo& OutInfo);
        static StructDefinition p_ParseStruct(const char* Data,size_t DataSize,size_t ParseOffset,size_t* OutParseOffset,LSPInfo& OutInfo);
        static MemberExpression p_ParseMemberExpression(const char* Data,size_t DataSize,size_t ParseOffset,size_t* OutParseOffset,bool IsLHS,int& CurrentLambdaID,std::vector<Lambda>& OutLambdas,LSPInfo& OutInfo);
        static std::vector<ParseRule> p_ParseParseRules(const char* Data,size_t DataSize,size_t ParseOffset,size_t* OutParseOffset,char EndMarker,int& CurrentLambdaID,std::vector<Lambda>& OutLambdas,LSPInfo& OutInfo);
        

        bool p_IsAssignable(StructIndex Lhs,StructIndex Rhs);
        TypeInfo p_GetMemberTypeInfo(StructDefinition const& StructScope,MemberExpression& Member,LSPInfo& OutInfo);
        TypeInfo p_GetRHSTypeInfo(MemberExpression& RHSExpression,int RHSMax,LSPInfo& OutInfo);
        bool p_IsAssignable(StructDefinition const& StructScope,MemberExpression& StructExpression,MemberExpression& RHSExpression,int LHSMax,LSPInfo& OutInfo);
    public:
        std::vector<Terminal> Terminals;
        std::vector<NonTerminal> NonTerminals;
        std::vector<StructDefinition> Structs;
        std::unordered_map<std::string,NonTerminalIndex> NameToNonTerminal;
        std::unordered_map<std::string,TerminalIndex> NameToTerminal;
        std::unordered_map<std::string,StructIndex> NameToStruct;
        
        DependancyInfo DepInfo; 
        
        std::string SkipRegex;

        static MBCCDefinitions ParseDefinitions(const char* Data,size_t DataSize,size_t ParseOffset);
        static MBCCDefinitions ParseDefinitions(const char* Data,size_t DataSize,size_t ParseOffset,LSPInfo& OutInfo);

        bool HasMember(StructDefinition const& StructDef,std::string const& Member);
        StructMemberVariable const* TryGetMember(StructDefinition const& StructDef,std::string const& Member) const;
        StructMemberVariable const& GetMember(StructDefinition const& StructDef,std::string const& Member) const;
        StructMemberVariable& GetMember(StructDefinition& StructDef,std::string const& Member);
    };
    DependancyInfo CalculateDependancyInfo(MBCCDefinitions const& Grammar);
    //Missing entry can be deduced by the bool being false
    class TerminalStringMap
    {
    private:
    public:
        TerminalStringMap(int k);
        bool& operator[](std::vector<TerminalIndex> const& Key);
        bool const& operator[](std::vector<TerminalIndex> const& Key) const;
    };
    class BoolTensor
    {
    private:
        std::vector<bool> m_Data;
        int m_J = 0;
        int m_K = 0;
    public:
        BoolTensor(int i,int j,int k);
        void SetValue(int i,int j,int k );
        bool GetValue(int i, int j, int k) const;
    };
    class GLA
    {
    private:
        typedef int NodeIndex;
        struct GLAEdge
        {
            GLAEdge() = default;
            GLAEdge(TerminalIndex TerminalIndex,NodeIndex ConnectionIndex)
            {
                ConnectionTerminal = TerminalIndex;   
                Connection = ConnectionIndex;
            }
            //-1 means that it's an E connection
            TerminalIndex ConnectionTerminal = -1;
            NodeIndex Connection = -1;
        };
        struct GLANode
        {
            GLANode(int k)
            {
                Visiting = std::vector<bool>(k);    
            }
            std::vector<bool> Visiting;
            std::vector<GLAEdge> Edges;
        };
        //The first size(NonTerm) is the Initial nodes, the following size(NonTerm) are the accepting nodes, and then 
        //the following are internal connections
        //Maybe replace with MBVector if the size of the GLA edges i relativly bounded?
        mutable std::vector<GLANode> m_Nodes;
        NonTerminalIndex m_NonTerminalCount = 0;
        TerminalIndex m_TerminalCount = 0;
        //std::vector<NodeIndex> m_ProductionBegin;

        std::vector<bool> p_LOOK(GLANode& Edge,int k) const;
    public:
        GLA(MBCCDefinitions const& Grammar,int k);
        //TODO optimize, currently exponential time algorithm
        MBMath::MBDynamicMatrix<bool> LOOK(NonTerminalIndex NonTerminal,int ProductionIndex,int k) const;
        //Used for A* components
        //MBMath::MBDynamicMatrix<bool> FIRST(NonTerminalIndex NonTerminal,int k);
        //BoolTensor CalculateFIRST();
        //BoolTensor CalculateFOLLOW();
    };
    class Tokenizer
    {
    private:      
        //Easy interfac, memeory map everything   
        size_t m_ParseOffset = 0;
        int m_LineOffset = 0;
        int m_LineByteOffset = 0;
        std::string m_TextData;
        std::regex m_Skip;
        std::vector<std::pair<std::regex,int>> m_TerminalRegexes;
        std::deque<Token> m_StoredTokens;
        Token p_ExtractToken();
        std::pair<int,int> p_GetLineAndPosition(size_t ParseOffset) const;  
    public: 
        Tokenizer(std::string const& SkipRegex,std::initializer_list<std::string> TerminalRegexes);
        void SetText(std::string NewText);
        void ConsumeToken();
        bool IsEOF(Token const& TokenToExamine);
        Token const& Peek(int Depth = 0);
        std::string GetPositionString() const;
    };
    class CPPStreamIndenter : public MBUtility::MBOctetOutputStream
    {
    private:       
        MBUtility::MBOctetOutputStream* m_AssociatedStream = nullptr;
        int m_IndentLevel = 0;
    public:
        CPPStreamIndenter(MBUtility::MBOctetOutputStream* StreamToConvert);
        size_t Write(const void* DataToWrite,size_t DataSize) override;
    };

    class ParsingException : public std::exception
    {
        std::string m_ErrorMessage;
    public: 
        TokenPosition Position; 
        std::string NonterminalName;
        std::string ExpectedType;
        ParsingException(ParsingException&&) noexcept = default;
        ParsingException(ParsingException const&) = default;
        ParsingException()
        {
               
        }
        ParsingException(TokenPosition NewPosition,std::string NewNonTerminal,std::string NewExpectedType)
            : NonterminalName(std::move(NewNonTerminal)),ExpectedType(std::move(NewExpectedType))
        {
            Position = NewPosition;
            m_ErrorMessage = "Error parsing "+NonterminalName+" at line "+std::to_string(NewPosition.Line)+", col "+
                std::to_string(NewPosition.Line)+" : expected "+ExpectedType;
        }
        const char* what() const noexcept override
        {
            return(m_ErrorMessage.c_str());
        }
    };




    bool Builtin(TypeInfo Info);
    MBMath::MBDynamicMatrix<bool> CombineProductions(std::vector<MBMath::MBDynamicMatrix<bool>> const& MatrixesToCombine);
    bool TypeIsBuiltin(std::string const& TypeToVerify);
    TypeInfo BuiltinToType(std::string const& BuiltinType);
    std::vector<bool> CalculateENonTerminals(MBCCDefinitions const& Grammar);
    bool RulesAreDisjunct(std::vector<MBMath::MBDynamicMatrix<bool>> const& ProductionsToVerify);
    class ParserCompiler
    {
    public:
        virtual void WriteParser(MBCCDefinitions const& Grammar,std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> const& TotalProductions,std::string const& OutputBase) = 0; 
    };
    struct GrammarOptions
    {
        int k = 1;
    };
    class ParserCompilerHandler
    {
    private:
        std::unordered_map<std::string,std::unique_ptr<ParserCompiler>> m_Compilers;
    public:
        ParserCompilerHandler();
        ParserCompilerHandler(ParserCompilerHandler const&) = delete;
        static void VerifyNotLeftRecursive(MBCCDefinitions const& Grammar,std::vector<bool> const& ERules);
        static std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> CalculateProductionsLinearApproxLOOK(MBCCDefinitions const& Grammar,std::vector<bool> const& ERules,GLA const& GrammarGLA,int k);
        static std::string Verify(MBCCDefinitions const& InfoToWrite);
        void WriteParser(MBCCDefinitions const& Grammar,GrammarOptions const& Options,std::string const& LanguageName,std::string const& OutputBase);
    };

    //class CPPParserGenerator
    //{
    //    std::vector<std::vector<std::string>> m_ProductionPredicates;

    //    void p_WriteParser(MBCCDefinitions const& Grammar,std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> const& ProductionsLOOk,
    //            std::string const& HeaderName,
    //            MBUtility::MBOctetOutputStream& HeaderOut,MBUtility::MBOctetOutputStream& SourceOut);
    //    void p_WriteHeader(MBCCDefinitions const& Grammar, MBUtility::MBOctetOutputStream& HeaderOut);
    //    void p_WriteFunctionHeaders(MBCCDefinitions const& Grammar,MBUtility::MBOctetOutputStream& HeaderOut);
    //    void p_WriteSource(MBCCDefinitions const& Grammar,std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> const& ProductionsLOOk,
    //            std::string const& HeaderName,MBUtility::MBOctetOutputStream& SourceOut);
    //    void p_WriteLOOKTable(std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> const& ProductionsLOOk,MBUtility::MBOctetOutputStream& SourceOut);
    //    std::string const& p_GetLOOKPredicate(NonTerminalIndex AssociatedNonTerminal,int Production = -1);
    //    void p_WriteNonTerminalFunction(MBCCDefinitions const& Grammar,NonTerminalIndex NonTerminal, MBUtility::MBOctetOutputStream& SourceOut);
    //    void p_WriteNonTerminalProduction(MBCCDefinitions const& Grammar,NonTerminalIndex NonTerminal,int ProductionIndex,std::string const& FunctionName,MBUtility::MBOctetOutputStream& SourceOut);

    //    static std::string p_GetTypeString(MBCCDefinitions const& Grammar,TypeInfo Type);
    //public:
    //    void WriteLLParser(MBCCDefinitions const& InfoToWrite,std::string const& HeaderName,MBUtility::MBOctetOutputStream& HeaderOut,MBUtility::MBOctetOutputStream& SourceOut,int k = 2);
    //};
}
