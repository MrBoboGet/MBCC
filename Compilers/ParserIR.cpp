#include "ParserIR.h"
#include <assert.h>

namespace MBCC
{
    Expr_GetVar h_GetLHS(MemberReference const& Var)
    {
        Expr_GetVar ReturnValue;
        for(int i = 0; i < Var.Names.size();i++)
        {
            if(i == 0 && Var.Names[i] == "this")
            {
                ReturnValue.Fields.push_back("ReturnValue");
            }
            else
            {
                ReturnValue.Fields.push_back(Var.Names[i]);
            }
        }
        return ReturnValue;
    }
    Expression h_GetRHS(MBCCDefinitions const& Grammar,RuleComponent const& Component)
    {
        Expression ReturnValue;
        auto const& Var = Component.ReferencedRule;
        if(Component.IsTerminal)
        {
            Expr_Convert Conversion;
            Conversion.ValueToConvert = std::make_unique<Expression>(Expr_PeekValue{0});
            Conversion.SuppliedValue = TypeFlags::String;
            Conversion.TargetValue = Component.AssignedMember.ResultType;
            return Conversion;
        }
        else if(Var.IsType<MemberReference>())
        {
            //parsing some non-terminal
            auto const& MemberRef = Var.GetType<MemberReference>();
            assert(MemberRef.Names[0] == "TOKEN" && MemberRef.Names.size() > 1 && "h_GetRHS only called with terminal / TOKEN value, otherwise value is filled");
            std::string const& MemberType  = MemberRef.Names[1];
            if(MemberType == "Position")
            {
                return Expr_PeekPosition();
            }
            else
            {
                assert(false && "h_GetRHS doesn't cover all cases");
            }
        }
        else if(Var.IsType<Literal>())
        {
            auto const& Lit = Var.GetType<Literal>();
            if(Component.ReferencedRule.ResultType & TypeFlags::String)
            {
                Expr_String String;
                String.Value = Lit.LiteralString;
                return String;
            }
            else if(Component.ReferencedRule.ResultType & TypeFlags::Bool)
            {
                Expr_Bool Bool;
                Bool.Value = Lit.LiteralString == "true";
                return Bool;
            }
            else if(Component.ReferencedRule.ResultType & TypeFlags::Int)
            {
                Expr_Integer Integer;
                Integer.Value = std::stoi(Lit.LiteralString);
                return Integer;
            }
            else
            {
                assert(false && "h_GetRHS in ParserIR doesn't cover all cases");
            }
        }
        else
        {
            assert(false && "h_GetRHS in ParserIR doesn't cover all cases");
        }
        return ReturnValue;
    }
    std::vector<Statement> h_GetComponentBody( MBCCDefinitions const& Grammar,
            std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> const& TotalProductions,
            NonTerminalIndex NonTermIndex,
            RuleComponent const& ComponentToWrite, 
            std::vector<Statement>& DelayedAssignments)
    {
        std::vector<Statement> ReturnValue;
        if(ComponentToWrite.IsInline)
        {
            std::vector<Statement_If> Rules;
            auto const& InlineNonTerm = Grammar.NonTerminals[ComponentToWrite.ComponentIndex];
            for(int i = 0; i < InlineNonTerm.Rules.size();i++)
            {
                Statement_If NewIf;
                NewIf.Condition = GetLookPredicate(Grammar,TotalProductions,ComponentToWrite.ComponentIndex,i);
                ConvertRuleBody(Grammar,Grammar.NonTerminals[ComponentToWrite.ComponentIndex],InlineNonTerm.Rules[i],NewIf.Content,DelayedAssignments);
                Rules.push_back(std::move(NewIf));
            }
            Statement_If ElseCase;
            ElseCase.Content = {Statement_Exception(Grammar.NonTerminals[NonTermIndex].Name,InlineNonTerm.Name)};
            Rules.push_back(std::move(ElseCase));
            Statement_IfChain Chain;
            Chain.Alternatives = std::move(Rules);
            ReturnValue.push_back(std::move(Chain));
            return ReturnValue;
        }
        bool IsSpecial = (ComponentToWrite.ReferencedRule.IsType<MemberReference>() && ComponentToWrite.ReferencedRule.GetType<MemberReference>().Names[0] == "TOKEN");
      
        //verify correct lookahead for better error messages
        
        if(!ComponentToWrite.ReferencedRule.IsEmpty())
        {
            if(ComponentToWrite.AssignedMember.IsEmpty())
            {
                if(!ComponentToWrite.IsTerminal && ComponentToWrite.ReferencedRule.IsType<MemberReference>())
                {
                    //we just parse some rule without assigning it to anything, calling for a 
                    //"direct" function call    
                    Statement_Expr ExprStatement;
                    Expr_ParseDirect Parse;
                    Parse.NonTerminal = ComponentToWrite.ComponentIndex;
                    ExprStatement.Expr = std::move(Parse);
                    ReturnValue.push_back(std::move(ExprStatement));
                }
                //a literal or token not assigned can just be ignored
            }
            else
            {
                auto LHS = h_GetLHS(ComponentToWrite.AssignedMember.GetType<MemberReference>());
                bool IsAssignment = IsSpecial || ComponentToWrite.IsTerminal || 
                    ComponentToWrite.ReferencedRule.GetType<MemberReference>().Names.size() > 1;
                //the result is assigned to something, either adding to a list or modifying a value
                if( ComponentToWrite.Max == -1 || 
                        ((ComponentToWrite.AssignedMember.ResultType & TypeFlags::List) && !(ComponentToWrite.ReferencedRule.ResultType & TypeFlags::List)))
                {
                    //assign list
                    if(IsAssignment)
                    {
                        Statement_AddList ListAdd;
                        ListAdd.Variable = std::move(LHS);
                        ListAdd.Value = h_GetRHS(Grammar,ComponentToWrite);
                        ReturnValue.push_back(std::move(ListAdd));
                    }
                    else
                    {
                        Statement_AddList ListAdd;
                        ListAdd.Variable = LHS;
                        ListAdd.Value = Expr_DefaultConstruct{Type(ComponentToWrite.ReferencedRule.ResultType)};
                        Statement_FillVar Fill;
                        Fill.ValueToFill = Expr_GetBack{LHS};
                        Fill.NonTerminal = ComponentToWrite.ComponentIndex;
                        Fill.FillType = Grammar.NonTerminals[ComponentToWrite.ComponentIndex].AssociatedStruct;
                        Fill.VarType = ComponentToWrite.AssignedMember.ResultType;
                        ReturnValue.push_back(std::move(ListAdd));
                    }
                }
                else
                {
                    if(IsAssignment)
                    {
                        Statement_AssignVar AssignVar;
                        AssignVar.Value = h_GetRHS(Grammar,ComponentToWrite);
                        AssignVar.Variable = std::move(LHS);
                        ReturnValue.push_back(std::move(AssignVar));
                    }
                    else
                    {
                        Statement_FillVar AssignVar;
                        AssignVar.ValueToFill = std::move(LHS);
                        AssignVar.NonTerminal = ComponentToWrite.ComponentIndex;
                        AssignVar.FillType = Grammar.NonTerminals[ComponentToWrite.ComponentIndex].AssociatedStruct;
                        AssignVar.VarType = ComponentToWrite.AssignedMember.ResultType;
                        ReturnValue.push_back(std::move(AssignVar));
                    }
                }
            }
        }
        else
        {
            assert(ComponentToWrite.AssignedMember.IsEmpty() && "Assigned member was non-empty while referenced rule was empty");   
        }
        if(ComponentToWrite.IsTerminal && !IsSpecial)
        {
            ReturnValue.push_back(Statement_PopToken());
        }
        return ReturnValue;
    }
    Expression h_GetComponentPredicate(LookType const& TotalProductions,MBCCDefinitions const& Grammar,RuleComponent const& Component)
    {
        Expression ReturnValue;
        if(Component.IsTerminal)
        {
            Expr_Equality TypeEq;
            Expr_PeekType ExprType;
            ExprType.PeekIndex = 0;
            Expr_Integer Integer;
            Integer.Value = Component.ComponentIndex;
            TypeEq.Lhs = std::make_unique<Expression>(std::move(ExprType));
            TypeEq.Rhs = std::make_unique<Expression>(std::move(Integer));
            return TypeEq;
        }
        else
        {
            return GetLookPredicate(Grammar,TotalProductions,Component.ComponentIndex,-1);
        }
        return ReturnValue;
    }
    void h_ConvertComponent( MBCCDefinitions const& Grammar,
            LookType const& TotalProductions,
            NonTerminalIndex NonTermIndex,
            RuleComponent const& ComponentToWrite, 
            std::vector<Statement>& OutStatements,
            std::vector<Statement>& DelayedAssignments)
    {
        auto const& AssociatedNonTerminal  = Grammar.NonTerminals[NonTermIndex];
        std::vector<Statement> Body = h_GetComponentBody(Grammar,TotalProductions,NonTermIndex,ComponentToWrite,DelayedAssignments);
        if(ComponentToWrite.Max == 1 && ComponentToWrite.Min == 0)
        {
            Statement_If NewStatement;
            NewStatement.Condition = h_GetComponentPredicate(TotalProductions,Grammar,ComponentToWrite);
            NewStatement.Content = std::move(Body);
            OutStatements.push_back(std::move(NewStatement));
        }
        else if(ComponentToWrite.Min == 0 && ComponentToWrite.Max == -1)
        {
            Statement_While NewStatement;
            NewStatement.Condition = h_GetComponentPredicate(TotalProductions,Grammar,ComponentToWrite);
            NewStatement.Content = std::move(Body);
            OutStatements.push_back(std::move(NewStatement));
        }
        else if(ComponentToWrite.Min == 1 && ComponentToWrite.Max == -1)
        {
            Statement_DoWhile NewStatement;
            NewStatement.Condition = h_GetComponentPredicate(TotalProductions,Grammar,ComponentToWrite);
            NewStatement.Content = std::move(Body);
            OutStatements.push_back(std::move(NewStatement));
        }
        else
        {
            //regular, just write the body and verify if terminal
            OutStatements.insert(OutStatements.end(),std::make_move_iterator(Body.begin()),std::make_move_iterator(Body.end()));
        }
    }
    void ConvertRuleBody( MBCCDefinitions const& Grammar,
            LookType  const& TotalProductions,
            NonTerminalIndex NonTermIndex,
            ParseRule const& Production,
            std::vector<Statement>& OutStatements,
            std::vector<Statement>& DelayedAssignments)
    {
        int RegularComponentCount = 0;
        int MetaComponentCount = 0;
        for(int i = 0; i < Production.MetaComponents.size() + Production.Components.size();i++)
        {
            RuleComponent const* ComponentPointer = nullptr;
            if(MetaComponentCount < Production.MetaComponents.size() && i == Production.MetaComponents[MetaComponentCount].first)
            {
                ComponentPointer = &Production.MetaComponents[MetaComponentCount].second;
                MetaComponentCount++;
            }
            else
            {
                ComponentPointer = &Production.Components[RegularComponentCount];
                RegularComponentCount++;   
            }
            auto const& Component = *ComponentPointer;
            h_ConvertComponent(Grammar,TotalProductions,NonTermIndex,*ComponentPointer,OutStatements,DelayedAssignments);
        }
    }
    std::vector<Statement> GetProductionContent(MBCCDefinitions const& Grammar,LookType const& TotalProductions, NonTerminalIndex TerminalIndex,int ProductionIndex)
    {
        std::vector<Statement> ReturnValue;
        std::vector<Statement> DelayedAssignments;

        return ReturnValue;
    }
    Function ConvertDirectionFunction(MBCCDefinitions const& Grammar,LookType const& TotalProductions,
            NonTerminalIndex TerminalIndex,int ProductionIndex)
    {
        Function ReturnValue;
        StructDefinition const* AssoicatedStruct = nullptr;
        NonTerminal const& AssociatedNonTerminal = Grammar.NonTerminals[TerminalIndex];
        ReturnValue.NonTerminal = TerminalIndex;
        if(AssociatedNonTerminal.AssociatedStruct != -1)
        {
            AssoicatedStruct = &Grammar.Structs[AssociatedNonTerminal.AssociatedStruct];    
        }
        if(AssoicatedStruct != nullptr)
        {
            ReturnValue.ReturnType.Name = AssoicatedStruct->Name;
        }
        Statement_DeclareVar DeclVar;
        DeclVar.Name = "ReturnValue";
        DeclVar.VarType = ReturnValue.ReturnType;
        Statement_FillVar FillVar;
        FillVar.ValueToFill = Expr_GetVar{{"ReturnValue"}};
        FillVar.NonTerminal = TerminalIndex;
        FillVar.ProductionIndex = ProductionIndex;
        

        Statement_Return ReturnVar;
        ReturnVar.Variable = "ReturnValue";
        ReturnValue.Content = {std::move(DeclVar),std::move(FillVar),std::move(ReturnVar)};
        return ReturnValue;
    }

    Statement_FillVar h_FillReturnValue(MBCCDefinitions const& Grammar,std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> const& TotalProductions,
            NonTerminalIndex TerminalIndex,int ProductionIndex)
    {
        Statement_FillVar ReturnValue;
        StructDefinition const* AssociatedStruct = nullptr;
        NonTerminal const& AssociatedNonTerminal = Grammar.NonTerminals[TerminalIndex];
        if(AssociatedNonTerminal.AssociatedStruct != -1)
        {
            AssociatedStruct = &Grammar.Structs[AssociatedNonTerminal.AssociatedStruct];    
        }
        else
        {
            assert(false && "Trying to fill empty struct");
        }
        ReturnValue.ValueToFill = Expr_GetVar{{"ReturnValue"}};
        ReturnValue.NonTerminal = TerminalIndex;
        ReturnValue.ProductionIndex = ProductionIndex;
        ReturnValue.FillType = AssociatedNonTerminal.AssociatedStruct;
        ReturnValue.VarType = AssociatedNonTerminal.AssociatedStruct;
        return ReturnValue;
    }
    Function ConvertFillFunction(MBCCDefinitions const& Grammar,LookType const& TotalProductions,
            NonTerminalIndex TerminalIndex,int ProductionIndex)
    {
        //void return value
        Function ReturnValue;
        ReturnValue.NonTerminal = TerminalIndex;
        ReturnValue.ProductionIndex = ProductionIndex;
        auto const& NonTerminal = Grammar.NonTerminals[TerminalIndex];
        if(ProductionIndex == -1)
        {
            if(NonTerminal.Rules.size() == 1)
            {
                ReturnValue.Content = GetProductionContent(Grammar,TotalProductions,TerminalIndex,0);
            } 
            Statement_IfChain Content;
            for(int i = 0; i < NonTerminal.Rules.size();i++)
            {
                Statement_If NewIf;
                NewIf.Condition = GetLookPredicate(Grammar,TotalProductions,TerminalIndex,i);
                NewIf.Content = {h_FillReturnValue(Grammar,TotalProductions,TerminalIndex,ProductionIndex)};
                Content.Alternatives.push_back(std::move(NewIf));
            }
            Statement_If ExceptionPath;
            Statement_Exception Exception;
            Exception.ExpectedName = NonTerminal.Name;
            Exception.NonTerminalName = NonTerminal.Name;
            ExceptionPath.Content =  {Exception};
            Content.Alternatives.push_back(ExceptionPath);
        }
        else
        {
            ReturnValue.Content = GetProductionContent(Grammar,TotalProductions,TerminalIndex,ProductionIndex);
        }

        return ReturnValue;
    }
    Expression GetLookPredicate(MBCCDefinitions const& Grammar,LookType const& TotalProductions,NonTerminalIndex NonTermIndex,int Production)
    {
        auto const& NonTerm = Grammar.NonTerminals[NonTermIndex];
        int k = TotalProductions[0][0].NumberOfColumns();
        if(Production == -1) Production = NonTerm.Rules.size();
        Expr_And ReturnValue;
        for(int i = 0; i < k; i++)
        {
            Expr_LOOKValue NewValue;
            NewValue.Indexes.push_back(Expr_Integer{NonTermIndex});
            NewValue.Indexes.push_back(Expr_Integer{Production});
            NewValue.Indexes.push_back(Expr_PeekType{i});
            ReturnValue.Arguments.push_back(std::move(NewValue));
        }
        return ReturnValue;
    }
    std::vector<Function> ConvertToIR(MBCCDefinitions const& Grammar,LookType const& TotalProductions)
    {
        std::vector<Function> ReturnValue;

        return ReturnValue;
    }
}
