# MBCC

MBCC is a parser generator, that is based on the paper [Obtaining practical variants of LL(k) and LR(k) for k>1 by splitting the atomic k-tuple](https://www.researchgate.net/publication/27233923_Obtaining_practical_variants_of_LLk_and_LRk_for_k1_by_splitting_the_atomic_k-tuple). 

It implements a parser generator specifically for the SLL<sup>1</sup>(k) grammar described in the paper. This means that the parser has linear execution time and intuitive grammar construction debuging, but can't handle as many grammars. It currently supports generating code only for C++. 

# Features

Not being designed to compete with more powerful generators, the main focus was to make a parser generator that can easily create an AST, and with features making it convenient to implement LSP features, such as line and character position for tokens. 

## First class AST creation

The AST is the main focus of mbcc, more or less all features are designed to ease the creation. The resulting AST structures are defined separately from the parse rules, and support common builtin types and operations related to AST creation. 

Single parent inheritance is supported, and the resulting AST is statically typed, with inspection only needed for polymorphic types. 

Below is an example of code defining a struct "Expr", that can be constructed through separate parse rules, and where derived structs can be assigned to base structs. 

```
term number = "[[:digit:]]+";
term operator = "+|-|*|/";
term lpar = "(";
term rpar = ")";
struct Expr
{
    
}
struct Operation : Expr
{
    string Operator;
    List<Expr> SubExpressions;
}
struct Literal : Expr
{
    string Value; 
}
def literal=Literal;
def operation=Operation;
def operation_b=Operation;
def expr=Expr;

literal = Value=number;
operation_b = SubExpressions=literal Operator=operator SubExpressions=expr;
operation = SubExpressions=expr Operator=operator SubExpressions=expr;
expr = this=literal | lpar this=expr rpar | this=operation;
```


Here the terminals are "number","operator","lpar" and "rpar". The structures are the expression structure, from which both the Literal and Operation structure inherit from. 

Non-terminals are defined separately, and are given a type with the "def" command, here the non-terminal literal is given the type Literal, operation is given the type Operation, and expr is given the type Expr. 

The use of polymorphism this way can also alleviate the grammar workarounds that pop-up from avoiding left recursive rules, which is highlighted here with the need to include the operation_b non-terminal. The resulting AST is however unchanged, as both operation_b and operation has the same type. 

## Special values for LSP creation

Implementing an LSP requires that some special values can be extracted from the AST in order to efficiently support some operations. Being able to support gotoDefinition for example requires that a position in the document can be translated into a corresponding structure that represents the token. This can be implemented with special values, an example given below. 

```
struct Type
{
    string Name;
    toenPos Position;
}
def type=Type;
type = Position=TOKEN.Position Name=idf;
```


Here the value of "TOKEN.Position" is the line + offset pair for the token at the current parsing state, that is the start of the next token in the token stream. This also allows for some efficient source translations can be implemented, as the beginning and end of a struct can be extracted with like this. 

```
foo = Begin=TOKEN.Position stuff=bar End=TOKEN.Position;
```


Where bar can be some arbitrarily complex rule. 

## Debugable output

Care is taken to ensure that the generated code is legible, and that it can realistically be debugged interactively. Sometimes errors in the grammar can be discovered by regular debugging, something that isn't really possible through table driven LL(k) parsers. 

# Documentation

Complete documentation for MBCC can be found [here](https://MrBoboGet.github.io/MBCC/index.html). 

# License

Everything in this repository is licensed under CC-zero. 

