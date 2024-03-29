#pragma once
#include "../MBCC.h"
#include "CLikeCompiler.h"
namespace MBCC
{
    class CSharpParserGenerator : public ParserCompiler,CLikeAdapter
    {
        std::string p_BuiltinToCSharp(TypeInfo BuiltinType);

        void p_WriteClasses(MBCCDefinitions const& Grammar,MBUtility::MBOctetOutputStream& OutStream);

        virtual std::string GetTypeString(MBCCDefinitions const& Grammar,TypeInfo Type) override;
        virtual std::string GetRHSString(MBCCDefinitions const& Grammar,RuleComponent const& ComponentToInspect) override;
        virtual std::string GetLHSMember(MBCCDefinitions const& Grammar,NonTerminal const& AssociatedNonTerminal
                ,ParseRule const& Production,RuleComponent const& ComponentToInspect) override;
        virtual std::string GetThrowExpectedException(std::string const& CurrentRule,std::string const& ExpectedRule) override;
        virtual std::string GetFunctionArguments() override;
        virtual std::string ListAddFunc() override;
        virtual std::string GetLookTableVariable(int TotalProductionSize,int LookDepth,int TerminalCount) override;
        virtual std::string GetLookIndex(std::string First,std::string Second, std::string Third) override;
        virtual std::string GetFunctionPrefix() override;
        virtual std::string VariableInitializationString(std::string const& TypeString) override;

        void p_WriteGetTokenizer(MBCCDefinitions const& Grammar,MBUtility::MBOctetOutputStream& OutStream);
    public:
        virtual void WriteParser(MBCCDefinitions const& Grammar,
                std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> const& TotalProductions,
                std::string const& OutputBase) override;
    };
}
