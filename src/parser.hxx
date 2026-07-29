#pragma once

#include "ast.hxx"
#include "lexeme.hxx"
#include "scanner.hxx"

#include <string>
#include <string_view>
#include <vector>

namespace basic {
class Parser {
public:
    Parser(Scanner& sc);
    ~Parser();

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
    Expression::Ptr parseExpression();
    Expression::Ptr parseAddition();
    Expression::Ptr parseMultiplication();
    Expression::Ptr parsePower();
    Expression::Ptr parseFactor();
    void parseNewLines();
    std::string match(Token tok);

private:
    Scanner& scanner;
    Lexeme lookahead;
};
} // basic
