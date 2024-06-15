#include "ParserIR.h"
#include <assert.h>

#include <numeric>
namespace MBCC
{
    std::string h_GetDelayedVarName(MemberReference const& Var)
    {
        std::string ReturnValue = std::accumulate(Var.Names.begin(),Var.Names.end(),std::string("i"),[](std::string const& lhs,std::string const& rhs){return lhs+"_"+rhs;});
        return ReturnValue;
    }
    Expr_GetVar h_GetLHS(TypeInfo BaseType,MemberReference const& Var,bool Delayed)
    {
        Expr_GetVar ReturnValue;
        if(Delayed)
        {
            ReturnValue.Fields.push_back(h_GetDelayedVarName(Var));
            ReturnValue.FieldTypes.push_back(BaseType);
            return ReturnValue;
        }
        ReturnValue.Fields.push_back("ReturnValue");
        ReturnValue.FieldTypes.push_back(BaseType);
        for(int i = 0; i < Var.Names.size();i++)
        {
            if(i == 0 && Var.Names[i] == "this")
            {
                assert(Var.Names.size() == 1);
                return ReturnValue;
            }
            else
            {
                ReturnValue.Fields.push_back(Var.Names[i]);
            }
            ReturnValue.FieldTypes.push_back(Var.PartTypes[i]);
        }
        return ReturnValue;
    }
    Expression h_GetRHS(MBCCDefinitions const& Grammar,RuleComponent const& Component,bool IsSpecial)
    {
        Expression ReturnValue;
        auto const& Var = Component.ReferencedRule;
        if(Component.IsTerminal && !IsSpecial)
        {
            if((Component.AssignedMember.ResultType & (~TypeFlags::List)) != TypeFlags::String)
            {
                Expr_Convert Conversion;
                Conversion.ValueToConvert = std::make_unique<Expression>(Expr_PeekValue{0});
                Conversion.SuppliedValue = TypeFlags::String;
                Conversion.TargetValue = (Component.AssignedMember.ResultType & (~TypeFlags::List));
                return Conversion;
            }
            else
            {
                return Expr_PeekValue();
            }
        }
        if(Var.IsType<MemberReference>())
        {
            //parsing some non-terminal
            auto const& MemberRef = Var.GetType<MemberReference>();
            if(MemberRef.Names[0] == "TOKEN")
            {
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
            else
            {
                Expr_ParseDirect Parse;
                Parse.NonTerminal = Component.ComponentIndex;
                for(int i = 1; i < MemberRef.Names.size();i++)
                {
                    Parse.SubFields.push_back(MemberRef.Names[i]);
                    Parse.FieldTypes.push_back(MemberRef.PartTypes[i]);
                }
                return Parse;
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
    Expression h_GetComponentPredicate(LookType const& TotalProductions,MBCCDefinitions const& Grammar,IROptions const& Options,std::vector<int> const& NonTermOffset,RuleComponent const& Component)
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
            return GetLookPredicate(Grammar,Options,TotalProductions,NonTermOffset,Component.ComponentIndex,-1);
        }
        return ReturnValue;
    }
    std::string h_GetTargetName(MBCCDefinitions const& Grammar,RuleComponent const& ComponentToVerify)
    {
        std::string ReturnValue;
        if(!ComponentToVerify.IsTerminal)
        {
            ReturnValue = Grammar.NonTerminals[ComponentToVerify.ComponentIndex].Name;
        }
        else
        {
            ReturnValue = Grammar.Terminals[ComponentToVerify.ComponentIndex].Name;
        }
        return ReturnValue;
    }
    Statement h_VerifyComponent(MBCCDefinitions const& Grammar,IROptions const& Options,LookType const& Look,std::vector<int> const& NonTermOffset,NonTerminalIndex NonTermIndex ,RuleComponent const& ComponentToVerify)
    {
        Statement_If CheckLook;
        CheckLook.Condition = h_GetComponentPredicate(Look,Grammar,Options,NonTermOffset,ComponentToVerify);
        if(CheckLook.Condition.IsType<Expr_And>()) CheckLook.Condition.GetType<Expr_And>().Negated = true;
        if(CheckLook.Condition.IsType<Expr_Equality>()) CheckLook.Condition.GetType<Expr_Equality>().Negated = true;
        CheckLook.Content.emplace_back(Statement_Exception(Grammar.NonTerminals[NonTermIndex].Name,h_GetTargetName(Grammar,ComponentToVerify)));
        return CheckLook;
    }
    std::vector<Statement> h_GetComponentBody( MBCCDefinitions const& Grammar,
            IROptions const& Options,
            LookType const& TotalProductions,
            std::vector<int> const& NonTermOffset,
            NonTerminalIndex NonTermIndex,
            ParseRule const& Rule, 
            RuleComponent const& ComponentToWrite, 
            std::unordered_map<std::string,DelayedAssignment>& DelayedAssignments)
    {
        std::vector<Statement> ReturnValue;
        if(ComponentToWrite.IsInline)
        {
            std::vector<Statement_If> Rules;
            auto const& InlineNonTerm = Grammar.NonTerminals[ComponentToWrite.ComponentIndex];
            for(int i = 0; i < InlineNonTerm.Rules.size();i++)
            {
                Statement_If NewIf;
                NewIf.Condition = GetLookPredicate(Grammar,Options,TotalProductions,NonTermOffset,ComponentToWrite.ComponentIndex,i);
                ConvertRuleBody(Grammar,Options,TotalProductions,NonTermOffset,ComponentToWrite.ComponentIndex,InlineNonTerm.Rules[i],NewIf.Content,DelayedAssignments);
                Rules.push_back(std::move(NewIf));
            }
            Statement_If ElseCase;
            ElseCase.Content.emplace_back(Statement_Exception(Grammar.NonTerminals[NonTermIndex].Name,InlineNonTerm.Name));
            Rules.push_back(std::move(ElseCase));
            Statement_IfChain Chain;
            Chain.Alternatives = std::move(Rules);
            ReturnValue.push_back(std::move(Chain));
            return ReturnValue;
        }
        bool IsSpecial = (ComponentToWrite.ReferencedRule.IsType<MemberReference>() && ComponentToWrite.ReferencedRule.GetType<MemberReference>().Names[0] == "TOKEN");
     
        if(ComponentToWrite.Min >= 1 && !IsSpecial && !ComponentToWrite.ReferencedRule.IsType<Literal>())
        {
            ReturnValue.push_back(h_VerifyComponent(Grammar,Options,TotalProductions,NonTermOffset,NonTermIndex,ComponentToWrite));
        }
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
                auto LHS = h_GetLHS(Grammar.NonTerminals[NonTermIndex].AssociatedStruct,ComponentToWrite.AssignedMember.GetType<MemberReference>(),false);
                bool IsAssignment = IsSpecial || ComponentToWrite.IsTerminal || ComponentToWrite.ReferencedRule.IsType<Literal>() ||
                    ComponentToWrite.ReferencedRule.GetType<MemberReference>().Names.size() > 1;

                if(ComponentToWrite.AssignOrder && Rule.NeedsAssignmentOrder && ComponentToWrite.AssignedMember.GetType<MemberReference>().Names[0] != "this")
                {
                    DelayedAssignment DelayedInfo;
                    DelayedInfo.Assignment.Variable = LHS;
                    LHS = h_GetLHS(Grammar.NonTerminals[NonTermIndex].AssociatedStruct,ComponentToWrite.AssignedMember.GetType<MemberReference>(),true);
                    DelayedInfo.Assignment.Value = LHS;
                    DelayedInfo.Variable.VarType = ComponentToWrite.AssignedMember.ResultType;
                    std::string VarName = h_GetDelayedVarName(ComponentToWrite.AssignedMember.GetType<MemberReference>());
                    DelayedInfo.Variable.Name = VarName;
                    DelayedAssignments[VarName] = std::move(DelayedInfo);
                }
                //the result is assigned to something, either adding to a list or modifying a value
                if( ComponentToWrite.Max == -1 || 
                        ((ComponentToWrite.AssignedMember.ResultType & TypeFlags::List) && !(ComponentToWrite.ReferencedRule.ResultType & TypeFlags::List)))
                {
                    //assign list
                    if(IsAssignment)
                    {
                        Statement_AddList ListAdd;
                        ListAdd.Variable = std::move(LHS);
                        ListAdd.Value = h_GetRHS(Grammar,ComponentToWrite,IsSpecial);
                        ReturnValue.push_back(std::move(ListAdd));
                    }
                    else
                    {
                        Statement_AddList ListAdd;
                        ListAdd.Variable = LHS;
                        ListAdd.Value = Expr_DefaultConstruct{ComponentToWrite.ReferencedRule.ResultType & TypeFlags::Base};
                        Statement_FillVar Fill;
                        Fill.ValueToFill = Expr_GetBack{LHS};
                        Fill.NonTerminal = ComponentToWrite.ComponentIndex;
                        Fill.FillType = ComponentToWrite.ReferencedRule.ResultType & TypeFlags::Base;
                        Fill.VarType = ComponentToWrite.AssignedMember.ResultType  & TypeFlags::Base;
                        ReturnValue.push_back(std::move(ListAdd));
                        ReturnValue.push_back(std::move(Fill));
                    }
                }
                else
                {
                    if(IsAssignment)
                    {
                        Statement_AssignVar AssignVar;
                        AssignVar.Value = h_GetRHS(Grammar,ComponentToWrite,IsSpecial);
                        AssignVar.Variable = std::move(LHS);
                        ReturnValue.push_back(std::move(AssignVar));
                    }
                    else
                    {
                        Statement_FillVar AssignVar;
                        AssignVar.ValueToFill = std::move(LHS);
                        AssignVar.NonTerminal = ComponentToWrite.ComponentIndex;
                        AssignVar.FillType = ComponentToWrite.ReferencedRule.ResultType & TypeFlags::Base;
                        AssignVar.VarType = ComponentToWrite.AssignedMember.ResultType & TypeFlags::Base;
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
    void h_ConvertComponent( MBCCDefinitions const& Grammar,
            IROptions const& Options,
            LookType const& TotalProductions,
            std::vector<int> const& NonTermOffset,
            NonTerminalIndex NonTermIndex,
            ParseRule const& Rule, 
            RuleComponent const& ComponentToWrite, 
            std::vector<Statement>& OutStatements,
            std::unordered_map<std::string,DelayedAssignment>& DelayedAssignments)
    {
        auto const& AssociatedNonTerminal  = Grammar.NonTerminals[NonTermIndex];
        std::vector<Statement> Body = h_GetComponentBody(Grammar,Options,TotalProductions,NonTermOffset,NonTermIndex,Rule,ComponentToWrite,DelayedAssignments);
        if(ComponentToWrite.Max == 1 && ComponentToWrite.Min == 0)
        {
            Statement_If NewStatement;
            NewStatement.Condition = h_GetComponentPredicate(TotalProductions,Grammar,Options,NonTermOffset,ComponentToWrite);
            NewStatement.Content = std::move(Body);
            OutStatements.push_back(std::move(NewStatement));
        }
        else if(ComponentToWrite.Min == 0 && ComponentToWrite.Max == -1)
        {
            Statement_While NewStatement;
            NewStatement.Condition = h_GetComponentPredicate(TotalProductions,Grammar,Options,NonTermOffset,ComponentToWrite);
            NewStatement.Content = std::move(Body);
            OutStatements.push_back(std::move(NewStatement));
        }
        else if(ComponentToWrite.Min == 1 && ComponentToWrite.Max == -1)
        {
            Statement_DoWhile NewStatement;
            NewStatement.Condition = h_GetComponentPredicate(TotalProductions,Grammar,Options,NonTermOffset,ComponentToWrite);
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
            IROptions const& Options,
            LookType  const& TotalProductions,
            std::vector<int> const& NonTermOffset,
            NonTerminalIndex NonTermIndex,
            ParseRule const& Production,
            std::vector<Statement>& OutStatements,
            std::unordered_map<std::string,DelayedAssignment>& DelayedAssignments)
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
            h_ConvertComponent(Grammar,Options,TotalProductions,NonTermOffset,NonTermIndex,Production,*ComponentPointer,OutStatements,DelayedAssignments);
        }
    }
    std::vector<Statement> GetProductionContent(MBCCDefinitions const& Grammar,IROptions const& Options,LookType const& TotalProductions, std::vector<int> const& NonTermOffset,NonTerminalIndex TerminalIndex,int ProductionIndex)
    {
        std::vector<Statement> ReturnValue;
        std::vector<Statement> Content;
        std::unordered_map<std::string,DelayedAssignment> DelayedAssignments;
        ConvertRuleBody(Grammar,Options,TotalProductions,NonTermOffset,TerminalIndex,Grammar.NonTerminals[TerminalIndex].Rules[ProductionIndex],Content,DelayedAssignments);

        for(auto& Assignment : DelayedAssignments)
        {
            ReturnValue.push_back(std::move(Assignment.second.Variable));
        }
        ReturnValue.insert(ReturnValue.end(),std::make_move_iterator(Content.begin()),
                std::make_move_iterator(Content.end()));
        for(auto& Assignment : DelayedAssignments)
        {
            ReturnValue.push_back(std::move(Assignment.second.Assignment));
        }
        return ReturnValue;
    }
    Function ConvertDirectionFunction(MBCCDefinitions const& Grammar,IROptions const& Options,LookType const& TotalProductions,
            std::vector<int> const& NonTermOffset,NonTerminalIndex TerminalIndex,int ProductionIndex)
    {
        Function ReturnValue;
        ReturnValue.Direct = true;
        StructDefinition const* AssociatedStruct = nullptr;
        NonTerminal const& AssociatedNonTerminal = Grammar.NonTerminals[TerminalIndex];
        ReturnValue.NonTerminal = TerminalIndex;
        ReturnValue.ProductionIndex = ProductionIndex;
        if(AssociatedNonTerminal.AssociatedStruct != -1)
        {
            AssociatedStruct = &Grammar.Structs[AssociatedNonTerminal.AssociatedStruct];    
        }
        if(AssociatedStruct != nullptr)
        {
            ReturnValue.ReturnType = AssociatedNonTerminal.AssociatedStruct;
        }
        Statement_DeclareVar DeclVar;
        DeclVar.Name = "ReturnValue";
        DeclVar.VarType = ReturnValue.ReturnType;
        ReturnValue.Content.emplace_back(std::move(DeclVar));

        if(Options.FillPolymorphic)
        {
            Statement_FillVar FillVar;
            FillVar.ValueToFill = Expr_GetVar{{"ReturnValue"},{AssociatedNonTerminal.AssociatedStruct}};
            FillVar.NonTerminal = TerminalIndex;
            FillVar.ProductionIndex = ProductionIndex;
            FillVar.FillType =  AssociatedNonTerminal.AssociatedStruct;
            FillVar.VarType =  AssociatedNonTerminal.AssociatedStruct;
            ReturnValue.Content.emplace_back(std::move(FillVar));
        }
        else
        {
            auto Content = GetFillFunctionContent(Grammar,Options,TotalProductions,NonTermOffset,TerminalIndex,-1);
            ReturnValue.Content.insert(ReturnValue.Content.end(),std::make_move_iterator(Content.begin()),std::make_move_iterator(Content.end()));
        }

        Statement_Return ReturnVar;
        ReturnVar.Variable = "ReturnValue";
        ReturnValue.Content.emplace_back(std::move(ReturnVar));
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
        ReturnValue.ValueToFill = Expr_GetVar{{"ReturnValue"},{AssociatedNonTerminal.AssociatedStruct}};
        ReturnValue.NonTerminal = TerminalIndex;
        ReturnValue.ProductionIndex = ProductionIndex;
        ReturnValue.FillType = AssociatedNonTerminal.AssociatedStruct;
        ReturnValue.VarType = AssociatedNonTerminal.AssociatedStruct;
        return ReturnValue;
    }

    std::vector<Statement> GetFillFunctionContent(MBCCDefinitions const& Grammar,IROptions const& Options,LookType const& TotalProductions,
            std::vector<int> const& NonTermOffset,NonTerminalIndex TerminalIndex,int ProductionIndex)
    {
        std::vector<Statement> ReturnValue;
        auto const& NonTerminal = Grammar.NonTerminals[TerminalIndex];
        if(ProductionIndex == -1)
        {
            if(NonTerminal.Rules.size() == 1)
            {
                ReturnValue = GetProductionContent(Grammar,Options,TotalProductions,NonTermOffset,TerminalIndex,0);
                return ReturnValue;
            } 
            Statement_IfChain Content;
            for(int i = 0; i < NonTerminal.Rules.size();i++)
            {
                Statement_If NewIf;
                NewIf.Condition = GetLookPredicate(Grammar,Options,TotalProductions,NonTermOffset,TerminalIndex,i);
                NewIf.Content.emplace_back(h_FillReturnValue(Grammar,TotalProductions,TerminalIndex,i));
                Content.Alternatives.push_back(std::move(NewIf));
            }
            Statement_If ExceptionPath;
            Statement_Exception Exception;
            Exception.ExpectedName = NonTerminal.Name;
            Exception.NonTerminalName = NonTerminal.Name;
            ExceptionPath.Content.emplace_back(Exception);
            Content.Alternatives.push_back(std::move(ExceptionPath));
            ReturnValue.push_back(std::move(Content));
        }
        else
        {
            ReturnValue = GetProductionContent(Grammar,Options,TotalProductions,NonTermOffset,TerminalIndex,ProductionIndex);
        }
        return ReturnValue;
    }
    Function ConvertFillFunction(MBCCDefinitions const& Grammar,IROptions const& Options,LookType const& TotalProductions,
            std::vector<int> const& NonTermOffset,NonTerminalIndex TerminalIndex,int ProductionIndex)
    {
        //void return value
        Function ReturnValue;
        ReturnValue.Direct = false;
        ReturnValue.NonTerminal = TerminalIndex;
        ReturnValue.ProductionIndex = ProductionIndex;
        auto const& NonTerminal = Grammar.NonTerminals[TerminalIndex];
        ReturnValue.ReturnType = NonTerminal.AssociatedStruct;
        ReturnValue.Content = GetFillFunctionContent(Grammar,Options,TotalProductions,NonTermOffset,TerminalIndex,ProductionIndex);
        return ReturnValue;
    }
    Expression GetLookPredicate(MBCCDefinitions const& Grammar,IROptions const& Options,LookType const& TotalProductions,
            std::vector<int> const& NonTermOffset,NonTerminalIndex NonTermIndex,int Production)
    {
        auto const& NonTerm = Grammar.NonTerminals[NonTermIndex];
        int k = TotalProductions[0][0].NumberOfColumns();
        if(Production == -1)
        {
            Production = 0;
        } 
        else
        {
            Production = Production+1;   
        }
        Expr_And ReturnValue;
        for(int i = 0; i < k; i++)
        {
            Expr_LOOKValue NewValue;
            NewValue.Indexes.push_back(Expr_Integer{NonTermOffset[NonTermIndex] + Production});
            NewValue.Indexes.push_back(Expr_Integer{i});
            NewValue.Indexes.push_back(Expr_PeekType{i});
            ReturnValue.Arguments.push_back(std::move(NewValue));
        }
        return ReturnValue;
    }
    std::vector<int> CalculateNonTermOffsets(LookType const& Look)
    {
        std::vector<int> ReturnValue;
        ReturnValue.reserve(Look.size());
        int CurrentOffset = 0;
        for(auto const& Production : Look)
        {
            ReturnValue.push_back(CurrentOffset);
            CurrentOffset += Production.size()+1;
        }
        return ReturnValue;
    }
    std::vector<Function> ConvertToIR(MBCCDefinitions const& Grammar,IROptions const& Options,LookType const& TotalProductions)
    {
        std::vector<Function> ReturnValue;
        auto NonTermOffset = CalculateNonTermOffsets(TotalProductions);
        for(int i = 0; i < Grammar.NonTerminals.size();i++)
        {
            auto const& NonTerm = Grammar.NonTerminals[i];
            if(NonTerm.IsInline)
            {
                continue;
            }
            for(int k = 0; k < NonTerm.Rules.size();k++)
            {
                ReturnValue.push_back(ConvertFillFunction(Grammar,Options,TotalProductions,NonTermOffset,i,k));
            }
            ReturnValue.push_back(ConvertFillFunction(Grammar,Options,TotalProductions,NonTermOffset,i,-1));
            ReturnValue.push_back(ConvertDirectionFunction(Grammar,Options,TotalProductions,NonTermOffset,i,-1));
        }
        return ReturnValue;
    }
}
