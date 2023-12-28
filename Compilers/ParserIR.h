#include "../MBCC.h"


#include <MBUtility/StaticVariant.h>
namespace MBCC
{
    struct Type
    {
        //either Name != "" or Info != -1
        std::string Name;
        TypeInfo Info = -1;

        Type(std::string name)
        {
            Name = name;   
        }
        Type(TypeInfo info)
        {
            Info = info;   
        }
        Type(){};
    };
    
    class Expression;
    struct Expr_LOOKValue
    {
        std::vector<Expression> Indexes;
    };
    struct Expr_GetVar
    {
        std::vector<std::string> Fields;
    };
    struct Expr_Integer
    {
        int Value = -1;
    };
    struct Expr_Bool
    {
        bool Value = false;
    };
    struct Expr_String
    {
        std::string Value;
    };
    struct Expr_ParseDirect
    {
        NonTerminalIndex NonTerminal = -1;
        int ProductionIndex = -1;
        std::vector<std::string> SubFields;
    };
    struct Expr_Convert
    {
        //must be a basic type
        TypeInfo TargetValue;
        TypeInfo SuppliedValue;
        std::unique_ptr<Expression> ValueToConvert;
    };
    //most likely only in appear in conditions
    struct Expr_And
    {
        bool Negated = false;
        std::vector<Expression> Arguments;
    };
    struct Expr_Equality
    {
        bool Negated = false;
        std::unique_ptr<Expression> Lhs;
        std::unique_ptr<Expression> Rhs;
    };
    struct Expr_PeekValue
    {
        int PeekIndex = 0;
    };
    struct Expr_PeekType
    {
        int PeekIndex = 0;
    };
    struct Expr_PeekPosition
    {
        int PeekIndex = 0;
    };
    struct Expr_DefaultConstruct
    {
        Type ObjectType;
    };
    struct Expr_GetBack
    {
        Expr_GetVar ListVariable;
    };

    class Expression : public MBUtility::StaticVariant< Expr_LOOKValue,Expr_GetVar,Expr_Integer,Expr_Bool,Expr_String,Expr_ParseDirect,Expr_Convert,Expr_And,Expr_Equality,Expr_PeekValue,Expr_PeekType,Expr_PeekPosition,Expr_DefaultConstruct,Expr_GetBack>
    {
    public:
        template<typename T,typename = InVariant<T>>
        Expression& operator=(T&& Data)
        {
            ((Base&)*this) = Data;
            return *this;
        }
        Expression() { }
        template<typename T,typename = InVariant<T>>
        Expression(T&& Data) : Base(std::forward<T>(Data))
        {
        }
    };

    class Statement;
    struct Statement_If
    {
        Expression Condition;
        std::vector<Statement> Content;
    };
    struct Statement_IfChain
    {
        //implicit else at the end
        std::vector<Statement_If> Alternatives;
    };
    struct Statement_While
    {
        Expression Condition;
        std::vector<Statement> Content;
    };
    struct Statement_DoWhile
    {
        Expression Condition;
        std::vector<Statement> Content;
    };
    struct Statement_AssignVar
    {
        Expr_GetVar Variable;
        Expression Value;
    };
    struct Statement_AddList
    {
        Expr_GetVar Variable;
        Expression Value;
    };
    struct Statement_Return
    {
        std::string Variable;
    };
    struct Statement_DeclareVar
    {
        Type VarType;
        std::string Name;

        Statement_DeclareVar(){}
        Statement_DeclareVar(Type type,std::string name)
        {
            VarType = type;
            Name = name;   
        }
    };
    struct Statement_Expr
    {
        Expression Expr;
    };
    struct Statement_Exception
    {
        std::string NonTerminalName;
        std::string ExpectedName;
        Statement_Exception(){};
        Statement_Exception(std::string TermName,std::string ExpName)
            : NonTerminalName(std::move(TermName)),ExpectedName(std::move(ExpName))
        {
            
        };
    };
    struct Statement_PopToken
    {
    };
    struct Statement_FillVar
    {
        NonTerminalIndex NonTerminal = -1;
        int ProductionIndex = -1;
        TypeInfo VarType = -1;
        TypeInfo FillType = -1;
        Expression ValueToFill;
    };
    class Statement : public MBUtility::StaticVariant<Statement_If,Statement_IfChain,Statement_While,Statement_DoWhile,Statement_AssignVar,Statement_AddList,Statement_Return,Statement_DeclareVar,Statement_Expr,Statement_Exception,Statement_PopToken,Statement_FillVar>
    {
    public:
        template<typename T,typename = InVariant<T>>
        Statement& operator=(T&& Data)
        {
            ((Base&)*this) = Data;
            return *this;
        }
        Statement() { }
        template<typename T,typename = InVariant<T>>
        Statement(T&& Data) : Base(std::forward<T>(Data))
        {
        }
    };

    struct Function
    {
        NonTerminalIndex NonTerminal = -1;
        int ProductionIndex = -1;
        //-1 == void
        Type ReturnType;
        std::vector<Statement> Content;
    };
    void ConvertRuleBody( MBCCDefinitions const& Grammar,
            NonTerminal const& AssociatedNonTerminal,
            ParseRule const& Production,
            std::vector<Statement>& OutStatements,
            std::vector<Statement>& DelayedAssignments);
   
    Expression GetLookPredicate(MBCCDefinitions const& Grammar,LookType const& TotalProductions,NonTerminalIndex NonTerminal,int Production);
    std::vector<Statement> GetProductionContent(MBCCDefinitions const& Grammar,std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> const& TotalProductions, NonTerminalIndex TerminalIndex,int ProductionIndex);
    Function ConvertDirectionFunction(MBCCDefinitions const& Grammar,LookType const& TotalProductions,
            NonTerminalIndex TerminalIndex,int ProductionIndex);
    Function ConvertFillFunction(MBCCDefinitions const& Grammar,LookType const& TotalProductions,
            NonTerminalIndex TerminalIndex,int ProductionIndex);
    std::vector<Function> ConvertToIR(MBCCDefinitions const& Grammar,LookType const& TotalProductions);
}
