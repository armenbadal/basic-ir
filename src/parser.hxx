#pragma once

#include "ast.hxx"
#include "lexeme.hxx"
#include "scanner.hxx"

#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>

namespace basic {
class Parser {
public:
    Parser(Scanner& sc);
    ~Parser();

    Program::Ptr parse();

    bool hasErrors() const noexcept;
    const std::vector<std::string>& getErrors() const noexcept;

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

    void parseNewLines();
    std::string match(Token tok);

    void reportError(const Lexeme& token, std::string_view message);
    void synchronize(std::initializer_list<Token> syncTokens);

private:
    Scanner& scanner;
    Lexeme lookahead;
    bool panicMode = false;
    std::vector<std::string> errors;
};
} // basic
