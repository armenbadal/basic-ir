#pragma once

#include "ast.hxx"
#include "diagnostics.hxx"
#include "lexeme.hxx"
#include "scanner.hxx"

#include <cstddef>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace basic {

class Parser {
public:
    Parser(Scanner& sc, Diagnostics& diag);
    ~Parser();

    // Վերլուծում է ամբողջ ծրագիրը
    //
    // Ուշադրություն. սխալի դեպքում վերլուծությունը չի ընդհատվում, և վերադարձված
    // ծառը թերի է լինում։ Կանչողը պարտավոր է նախ ստուգել Diagnostics-ը և
    // միայն սխալների բացակայության դեպքում ծառը փոխանցել հաջորդ փուլերին։
    Program::Ptr parse();

private:
    Program::Ptr parseProgram();
    Subroutine::Ptr parseSubroutine();

    Statement::Ptr parseOneStatement();
    Sequence::Ptr parseSequence();
    Let::Ptr parseLet();
    Dim::Ptr parseDim();
    Input::Ptr parseInput();
    Print::Ptr parsePrint();
    If::IfThen::Ptr parseIfThen(bool first);
    If::Ptr parseIf();
    While::Ptr parseWhile();
    For::Ptr parseFor();
    Call::Ptr parseCall();
    void parseNewLines();
    void parseBlockEnd(Token keyword);

    std::vector<Expression::Ptr> parseExpressionList();
    Expression::Ptr parseExpression();
    Expression::Ptr parseAddition();
    Expression::Ptr parseMultiplication();
    Expression::Ptr parsePower();
    Expression::Ptr parseUnary();
    Expression::Ptr parseSubscript();
    Expression::Ptr parseFactor();
    Boolean::Ptr parseTrueOrFalse();
    Number::Ptr parseNumber();
    Text::Ptr parseText();
    Expression::Ptr parseArrayDefinition();
    Expression::Ptr parseIdentOrApply();
    Expression::Ptr parseGrouped();

    void advance();
    std::string match(Token tok);

    void sync(const std::set<Token>& stops, std::string_view message);

private:
    Scanner& scanner;
    Diagnostics& diagnostics;

    Lexeme lookahead;

};
} // basic
