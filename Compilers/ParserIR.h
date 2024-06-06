#pragma once
#include "../MBCC.h"


#include <MBUtility/StaticVariant.h>
#include <type_traits>

#include <assert.h>
namespace MBCC
{
    class Expression;
    struct Expr_LOOKValue
    {
        std::vector<Expression> Indexes;
    };
    struct Expr_GetVar
    {
        std::vector<std::string> Fields;
        std::vector<TypeInfo> FieldTypes;
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
        std::vector<TypeInfo> FieldTypes;
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
        TypeInfo ObjectType;
    };
    struct Expr_GetBack
    {
        Expr_GetVar ListVariable;
    };

    class Expression : public MBUtility::StaticVariant<std::monostate,Expr_LOOKValue,Expr_GetVar,Expr_Integer,Expr_Bool,Expr_String,Expr_ParseDirect,Expr_Convert,Expr_And,Expr_Equality,Expr_PeekValue,Expr_PeekType,Expr_PeekPosition,Expr_DefaultConstruct,Expr_GetBack>
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
        std::vector<Statement> Alternatives;
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
        TypeInfo VarType;
        std::string Name;

        Statement_DeclareVar(){}
        Statement_DeclareVar(TypeInfo type,std::string name)
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
        Statement(Statement const&) = delete;
        Statement& operator=(Statement const&) = delete;
        Statement& operator=(Statement&&) noexcept = default;
        Statement(Statement &&) noexcept = default;
        template<typename T,typename = InVariant<T>>
        Statement& operator=(T&& Data)
        {
            ((Base&)*this) = std::forward<T>(Data);
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
        TypeInfo ReturnType = -1;
        bool Direct = false;
        std::vector<Statement> Content;
    };
    struct DelayedAssignment
    {
        Statement_DeclareVar Variable;
        Statement_AssignVar Assignment;
    };
    void ConvertRuleBody( MBCCDefinitions const& Grammar,
            LookType  const& TotalProductions,
            std::vector<int> const& NonTermOffset,
            NonTerminalIndex NonTermIndex,
            ParseRule const& Production,
            std::vector<Statement>& OutStatements,
            std::unordered_map<std::string,DelayedAssignment>& DelayedAssignments);
    Expression GetLookPredicate(
            MBCCDefinitions const& Grammar,
            LookType const& TotalProductions,
            std::vector<int> const& NonTermOffset,
            NonTerminalIndex NonTerminal,
            int Production);
    std::vector<Statement> GetProductionContent(
            MBCCDefinitions const& Grammar,
            LookType const& TotalProductions, 
            std::vector<int> const& NonTermOffset,
            NonTerminalIndex TerminalIndex,
            int ProductionIndex);
    Function ConvertDirectionFunction(MBCCDefinitions const& Grammar,
            LookType const& TotalProductions,
            std::vector<int> const& NonTermOffset,
            NonTerminalIndex TerminalIndex,
            int ProductionIndex);
    Function ConvertFillFunction(MBCCDefinitions const& Grammar,
            LookType const& TotalProductions,
            std::vector<int> const& NonTermOffset,
            NonTerminalIndex TerminalIndex,
            int ProductionIndex);
    std::vector<int> CalculateNonTermOffsets(LookType const& Look);
    std::vector<Function> ConvertToIR(MBCCDefinitions const& Grammar,LookType const& TotalProductions);


    template<typename T,typename... Args>
    constexpr bool Callable_v = std::is_invocable_v<T,Args...>;

    template<typename T,typename... Args>
    constexpr bool BoolResult_v = std::is_same_v<bool,decltype(std::declval<T>()(std::declval<Args>()...))>;

    template<typename T,typename... Args>
    bool VisitAndPoll(T& Visitor,Args const&... Arguments)
    {
        bool ReturnValue = true;
        if constexpr(Callable_v<T,Args...>)
        {
            if constexpr(BoolResult_v<T,Args...>)
            {
                if(!Visitor(Arguments...))
                {
                    return false;
                }    
            }
            else
            {
                Visitor(Arguments...);
            }
        }
        return ReturnValue;
    }
    //MBUtility::StaticVariant< Expr_LOOKValue,Expr_GetVar,Expr_Integer,Expr_Bool,Expr_String,Expr_ParseDirect,Expr_Convert,Expr_And,Expr_Equality,Expr_PeekValue,Expr_PeekType,Expr_PeekPosition,Expr_DefaultConstruct,Expr_GetBack>
    template<typename T,typename S, typename = std::enable_if_t<std::conjunction_v<std::negation<std::is_same<S,Expression>>,
        std::is_constructible<Expression,S>>>>
    void Traverse(T& Visitor,S const& ExpressionToTraverse)
    {
        //bool NotMonoState = !std::is_same_v<std::monostate,S>;
        //assert(NotMonoState && "Empty expression traversed");
        if constexpr(std::is_same_v<S,Expr_LOOKValue>)
        {
            auto const& Look = static_cast<Expr_LOOKValue const&>(ExpressionToTraverse);
            if(!VisitAndPoll(Visitor,Look)) return;
        }
        else if constexpr(std::is_same_v<S,Expr_GetVar>)
        {
            auto const& Look = static_cast<Expr_GetVar const&>(ExpressionToTraverse);
            if(!VisitAndPoll(Visitor,Look)) return;
        }
        else if constexpr(std::is_same_v<S,Expr_Integer>)
        {
            auto const& Look = static_cast<Expr_Integer const&>(ExpressionToTraverse);
            if(!VisitAndPoll(Visitor,Look)) return;
        }
        else if constexpr(std::is_same_v<S,Expr_Bool>)
        {
            auto const& Look = static_cast<Expr_Bool const&>(ExpressionToTraverse);
            if(!VisitAndPoll(Visitor,Look)) return;
        }
        else if constexpr(std::is_same_v<S,Expr_String>)
        {
            auto const& Look = static_cast<Expr_String const&>(ExpressionToTraverse);
            if(!VisitAndPoll(Visitor,Look)) return;
        }
        else if constexpr(std::is_same_v<S,Expr_ParseDirect>)
        {
            auto const& Look = static_cast<Expr_ParseDirect const&>(ExpressionToTraverse);
            if(!VisitAndPoll(Visitor,Look)) return;
        }
        else if constexpr(std::is_same_v<S,Expr_Convert>)
        {
            auto const& Convert = static_cast<Expr_Convert const&>(ExpressionToTraverse);
            if(!VisitAndPoll(Visitor,Convert)) return;
            Traverse(Visitor,*Convert.ValueToConvert);
        }
        else if constexpr(std::is_same_v<S,Expr_And>)
        {
            auto const& Convert = static_cast<Expr_And const&>(ExpressionToTraverse);
            if(!VisitAndPoll(Visitor,Convert)) return;
            for(auto const& Args : Convert.Arguments)
            {
                Traverse(Visitor,Args);
            }
        }
        else if constexpr(std::is_same_v<S,Expr_Equality>)
        {
            auto const& Equality = static_cast<Expr_Equality const&>(ExpressionToTraverse);
            if(!VisitAndPoll(Visitor,Equality)) return;
            Traverse(Visitor,*Equality.Lhs);
            Traverse(Visitor,*Equality.Rhs);
        }
        else if constexpr(std::is_same_v<S,Expr_PeekValue>)
        {
            auto const& Equality = static_cast<Expr_PeekValue const&>(ExpressionToTraverse);
            if(!VisitAndPoll(Visitor,Equality)) return;
        }
        else if constexpr(std::is_same_v<S,Expr_PeekType>)
        {
            auto const& Equality = static_cast<Expr_PeekType const&>(ExpressionToTraverse);
            if(!VisitAndPoll(Visitor,Equality)) return;
        }
        else if constexpr(std::is_same_v<S,Expr_PeekPosition>)
        {
            auto const& Equality = static_cast<Expr_PeekPosition const&>(ExpressionToTraverse);
            if(!VisitAndPoll(Visitor,Equality)) return;
        }
        else if constexpr(std::is_same_v<S,Expr_DefaultConstruct>)
        {
            auto const& Equality = static_cast<Expr_DefaultConstruct const&>(ExpressionToTraverse);
            if(!VisitAndPoll(Visitor,Equality)) return;
        }
        else if constexpr(std::is_same_v<S,Expr_GetBack>)
        {
            auto const& Equality = static_cast<Expr_GetBack const&>(ExpressionToTraverse);
            if(!VisitAndPoll(Visitor,Equality)) return;
            Traverse(Visitor,Equality.ListVariable);
        }
    }
    template<typename T>
    void Traverse(T& Visitor,Expression const& ExpressionToTraverse)
    {
        if(!VisitAndPoll(Visitor,ExpressionToTraverse))
        {
            return;   
        }
        auto VisitorAdapter = [&](auto const& Val){Traverse(Visitor,Val);};
        ExpressionToTraverse.Visit(VisitorAdapter);
    }

    //MBUtility::StaticVariant<Statement_If,Statement_IfChain,Statement_While,Statement_DoWhile,Statement_AssignVar,Statement_AddList,Statement_Return,Statement_DeclareVar,Statement_Expr,Statement_Exception,Statement_PopToken,Statement_FillVar>
    template<typename T>
    void Traverse(T& Visitor,Statement const& StatementToTraverse)
    {
        if(!VisitAndPoll(Visitor,StatementToTraverse))
        {
            return;   
        }
        if(StatementToTraverse.IsType<Statement_If>())
        {
            auto const& If = StatementToTraverse.GetType<Statement_If>();
            if(!VisitAndPoll(Visitor,If)) return;
            Traverse(Visitor,If.Condition);
            for(auto const& Statement : If.Content)
            {
                Traverse(Visitor,Statement);
            }
        }
        else if(StatementToTraverse.IsType<Statement_IfChain>())
        {
            auto const& If = StatementToTraverse.GetType<Statement_IfChain>();
            if(!VisitAndPoll(Visitor,If)) return;
            for(auto const& Statement : If.Alternatives)
            {
                Traverse(Visitor,Statement);
            }
        }
        else if(StatementToTraverse.IsType<Statement_While>())
        {
            auto const& While = StatementToTraverse.GetType<Statement_While>();
            if(!VisitAndPoll(Visitor,While)) return;
            Traverse(Visitor,While.Condition);
            for(auto const& Statement : While.Content)
            {
                Traverse(Visitor,Statement);
            }
        }
        else if(StatementToTraverse.IsType<Statement_DoWhile>())
        {
            auto const& While = StatementToTraverse.GetType<Statement_DoWhile>();
            if(!VisitAndPoll(Visitor,While)) return;
            Traverse(Visitor,While.Condition);
            for(auto const& Statement : While.Content)
            {
                Traverse(Visitor,Statement);
            }
        }
        else if(StatementToTraverse.IsType<Statement_AssignVar>())
        {
            auto const& Assignment = StatementToTraverse.GetType<Statement_AssignVar>();
            if(!VisitAndPoll(Visitor,Assignment)) return;
            Traverse(Visitor,Assignment.Variable);
            Traverse(Visitor,Assignment.Value);
        }
        else if(StatementToTraverse.IsType<Statement_AddList>())
        {
            auto const& Add = StatementToTraverse.GetType<Statement_AddList>();
            if(!VisitAndPoll(Visitor,Add)) return;
            Traverse(Visitor,Add.Variable);
            Traverse(Visitor,Add.Value);
        }
        else if(StatementToTraverse.IsType<Statement_Return>())
        {
            auto const& Return = StatementToTraverse.GetType<Statement_Return>();
            if(!VisitAndPoll(Visitor,Return)) return;
        }
        else if(StatementToTraverse.IsType<Statement_DeclareVar>())
        {
            auto const& Declaration = StatementToTraverse.GetType<Statement_DeclareVar>();
            if(!VisitAndPoll(Visitor,Declaration)) return;
        }
        else if(StatementToTraverse.IsType<Statement_Expr>())
        {
            auto const& Expression = StatementToTraverse.GetType<Statement_Expr>();
            if(!VisitAndPoll(Visitor,Expression)) return;
            Traverse(Visitor,Expression.Expr);
        }
        else if(StatementToTraverse.IsType<Statement_Exception>())
        {
            auto const& Exception = StatementToTraverse.GetType<Statement_Exception>();
            if(!VisitAndPoll(Visitor,Exception)) return;
        }
        else if(StatementToTraverse.IsType<Statement_PopToken>())
        {
            auto const& Pop = StatementToTraverse.GetType<Statement_PopToken>();
            if(!VisitAndPoll(Visitor,Pop)) return;
        }
        else if(StatementToTraverse.IsType<Statement_FillVar>())
        {
            auto const& Fill = StatementToTraverse.GetType<Statement_FillVar>();
            if(!VisitAndPoll(Visitor,Fill)) return;
            Traverse(Visitor,Fill.ValueToFill);
        }
        else
        {
            assert(false && "Statement traverse doesn't cover all cases");
        }
    }
    
    template<typename T>
    void Traverse(T& Visitor,Function const& FunctionToTraverse)
    {
        if(!VisitAndPoll(Visitor,FunctionToTraverse))
        {
            return;   
        }
        for(auto const& Statement : FunctionToTraverse.Content)
        {
            Traverse(Visitor,Statement);
        }
    }
}
