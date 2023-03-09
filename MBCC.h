#pragma once
#include <stdint.h>
#include <string>
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
namespace MBCC
{
    class MemberVariable
    {
    public:
        std::string Name; 
        std::string DefaultValue;
    };
    class StructMemberVariable_List : public MemberVariable
    {
    public:
        std::string ListType;
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
        std::string& GetDefaultValue();
        std::string const& GetName() const;
        std::string const& GetDefaultValue() const;
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
        std::string Name; 
        std::string ParentStruct;
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

    struct MemberExpression
    {
        std::vector<TypeInfo> PartTypes;
        std::vector<std::string> Names;
    };
    struct RuleComponent
    {
        bool IsTerminal = false;
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
            }
            else if(Line == Rhs.Line)
            {
                ReturnValue = ByteOffset < Rhs.ByteOffset;
            }
            return(ReturnValue);
        }
    };
    struct Token
    {
        TerminalIndex Type = -1;
        size_t ByteOffset;
        TokenPosition Position;
        std::string Value;    
    };
    struct DependancyInfo
    {
        std::vector<StructIndex> StructureDependancyOrder; 
        //Would it be faster to use a matrix?
        std::vector<std::set<StructIndex>> ChildrenMap;
    };
    class MBCCDefinitions
    {
    private:
        void p_VerifyStructs();
        //MEGA ugly, refactor
        void p_VerifyComponent(RuleComponent& ComponentToVerify,std::string const& NonTerminalName,StructDefinition const* AssociatedStruct,bool& ThisAssignment,bool& RegularAssignment);
        void p_VerifyRules();
        void p_UpdateReferencesAndVerify();

        static std::string p_ParseIdentifier(const char* Data,size_t DataSize,size_t ParseOffset,size_t* OutParseOffset);
        static SemanticAction p_ParseSemanticAction(const char* Data,size_t DataSize,size_t ParseOffset,size_t* OutParseOffset);
        static Terminal p_ParseTerminal(const char* Data,size_t DataSize,size_t ParseOffset,size_t* OutParseOffset);
        static std::pair<std::string,std::string> p_ParseDef(const char* Data,size_t DataSize,size_t ParseOffset,size_t* OutParseOffset);
        static StructMemberVariable p_ParseMemberVariable(const char* Data,size_t DataSize,size_t ParseOffset,size_t* OutParseOffset);
        static StructDefinition p_ParseStruct(const char* Data,size_t DataSize,size_t ParseOffset,size_t* OutParseOffset);
        static MemberExpression p_ParseMemberExpression(const char* Data,size_t DataSize,size_t ParseOffset,size_t* OutParseOffset);
        static std::vector<ParseRule> p_ParseParseRules(const char* Data,size_t DataSize,size_t ParseOffset,size_t* OutParseOffset);

        

        bool p_IsAssignable(StructIndex Lhs,StructIndex Rhs);
        TypeInfo p_GetMemberTypeInfo(StructDefinition const& StructScope,MemberExpression& Member,std::string& OutError);
        TypeInfo p_GetRHSTypeInfo(MemberExpression& RHSExpression,int RHSMax,std::string& OutError);
        bool p_IsAssignable(StructDefinition const& StructScope,MemberExpression& StructExpression,MemberExpression& RHSExpression,int LHSMax,std::string& OutError);
    public:
        std::vector<Terminal> Terminals;
        std::vector<NonTerminal> NonTerminals;
        std::vector<StructDefinition> Structs;
        std::unordered_map<std::string,NonTerminalIndex> NameToNonTerminal;
        std::unordered_map<std::string,TerminalIndex> NameToTerminal;
        std::unordered_map<std::string,StructIndex> NameToStruct;
        
        DependancyInfo DepInfo; 
        
        std::string SkipRegex;
        NonTerminalIndex Start = -1;

        static MBCCDefinitions ParseDefinitions(const char* Data,size_t DataSize,size_t ParseOffset);
        static MBCCDefinitions ParseDefinitions(const char* Data,size_t DataSize,size_t ParseOffset,std::string& OutError);

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
            //-1 means that it's and E connection
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
        int m_LineOffset = 1;
        int m_LineByteOffset = 0;
        std::string m_TextData;
        std::regex m_Skip;
        std::vector<std::regex> m_TerminalRegexes;
        std::deque<Token> m_StoredTokens;
        Token p_ExtractToken();
        std::pair<int,int> p_GetLineAndPosition(size_t ParseOffset) const;  
    public: 
        Tokenizer(std::string const& SkipRegex,std::initializer_list<std::string> TerminalRegexes);
        void SetText(std::string NewText);
        void ConsumeToken();
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

    std::vector<bool> CalculateENonTerminals(MBCCDefinitions const& Grammar);
    class LLParserGenerator
    {
        static std::vector<bool> p_RetrieveENonTerminals(MBCCDefinitions const& Grammar);
        static void p_VerifyNotLeftRecursive(MBCCDefinitions const& Grammar,std::vector<bool> const& ERules);
        //Non Terminal X Non Terminal Size
        //Based on "LL LK requries k > 1, and in turn on the Linear-approx-LL(k) algorithm. If I understand the 
        //algoritm correctly however, so might it be a bit of an pessimisation, as NonTermFollow of a NonTerminal 
        //Is actually dependant on the specific rule being processed
        //Non Terminal x K x Terminal 
        //TerminalIndex = -1 sentinel for empty rule, +1 for empty, +2 for EOF
        void p_WriteDefinitions(MBCCDefinitions const& Grammar,std::vector<TerminalStringMap> const& ParseTable,MBUtility::MBOctetOutputStream& HeaderOut,MBUtility::MBOctetOutputStream& SourceOut, int k);
        void p_WriteParser(MBCCDefinitions const& Grammar,std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> const& ProductionsLOOk,
                std::string const& HeaderName,
                MBUtility::MBOctetOutputStream& HeaderOut,MBUtility::MBOctetOutputStream& SourceOut);
        void p_WriteHeader(MBCCDefinitions const& Grammar, MBUtility::MBOctetOutputStream& HeaderOut);
        void p_WriteFunctionHeaders(MBCCDefinitions const& Grammar,MBUtility::MBOctetOutputStream& HeaderOut);
        void p_WriteSource(MBCCDefinitions const& Grammar,std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> const& ProductionsLOOk,
                std::string const& HeaderName,MBUtility::MBOctetOutputStream& SourceOut);
        void p_WriteLOOKTable(std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> const& ProductionsLOOk,MBUtility::MBOctetOutputStream& SourceOut);
        std::string const& p_GetLOOKPredicate(NonTerminalIndex AssociatedNonTerminal,int Production = -1);
        std::vector<std::vector<std::string>> m_ProductionPredicates;
        void p_WriteNonTerminalFunction(MBCCDefinitions const& Grammar,NonTerminalIndex NonTerminal, MBUtility::MBOctetOutputStream& SourceOut);
        void p_WriteNonTerminalProduction(MBCCDefinitions const& Grammar,NonTerminalIndex NonTerminal,int ProductionIndex,std::string const& FunctionName,MBUtility::MBOctetOutputStream& SourceOut);


        static std::string p_GetTypeString(MBCCDefinitions const& Grammar,TypeInfo Type);
        //Could possibly cache result
        //-1 to specify full look
    public:
        static void VerifyNotLeftRecursive(MBCCDefinitions const& Grammar,std::vector<bool> const& ERules);
        static std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> CalculateProductionsLinearApproxLOOK(MBCCDefinitions const& Grammar,std::vector<bool> const& ERules,GLA const& GrammarGLA,int k);
        static std::string Verify(MBCCDefinitions const& InfoToWrite);
        void WriteLLParser(MBCCDefinitions const& InfoToWrite,std::string const& HeaderName,MBUtility::MBOctetOutputStream& HeaderOut,MBUtility::MBOctetOutputStream& SourceOut,int k = 2);
    };
}
