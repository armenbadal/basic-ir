
#include "Parser.h"

namespace basic {
///
Parser::Parser(Scanner& scan)
    :scanner{ scan }
{}

///
void Parser::parseProgram()
{
    while (true) {
        if (lookahead.is(Token::Declare))
            parseDeclare();
        else if (lookahead.is(Token::Function))
            parseFunction();
        else
            break;
    }
}

///
void Parser::parseDeclare()
{
    match(Token::Declare);
    parseHeader();
}

///
void Parser::parseFunction()
{
    parseHeader();
    //
    match(Token::End);
    match(Token::Function);
}

///
void Parser::parseHeader()
{
    match(Token::Function);
    match(Token::Identifier);
    match(Token::LeftPar);
    if (lookahead.is(Token::Identifier)) {
        match(Token::Identifier);
        while (lookahead.is(Token::Comma)) {
            match(Token::Comma);
            match(Token::Identifier);
        }
    }
    match(Token::RightPar);
    parseNewLines();
}

///
void Parser::parseCommand()
{
    while (true) {
        if (lookahead.is(Token::Input))
            parseInput();
        else if (lookahead.is(Token::Print))
            parsePrint();
        else if (lookahead.is(Token::Let))
            parseLet();
        else if (lookahead.is(Token::Identifier))
            parseLet();
        else if (lookahead.is(Token::If))
            parseIf();
        else if (lookahead.is(Token::While))
            parseWhile();
        else if (lookahead.is(Token::For))
            parseFor();
        else if (lookahead.is(Token::Call))
            parseCall();
        else
            break;
        parseNewLines();
    }
}

///
void Parser::parseInput()
{
    match(Token::Input);
    match(Token::Identifier);
}

//
void Parser::parsePrint()
{
    match(Token::Print);
    // TODO parse expression
}

void Parser::parseLet()
{
    if (lookahead.is(Token::Let))
        scanner >> lookahead;
    match(Token::Identifier);
    match(Token::Eq);
    // TODO parse expression
}

///
void Parser::parseIf()
{
    match(Token::If);
    // TODO parse expression
    match(Token::Then);
    parseNewLines();
    parseCommand();
    while (lookahead.is(Token::ElseIf)) {
        match(Token::ElseIf);
        // TODO parse expression
        match(Token::Then);
        parseNewLines();
        parseCommand();
    }
    if (lookahead.is(Token::Else)) {
        match(Token::Else);
        parseNewLines();
        parseCommand();
    }
    match(Token::End);
    match(Token::If);
}

///
void Parser::parseWhile()
{
    match(Token::While);
    // TODO parse expression
    parseNewLines();
    parseCommand();
    match(Token::End);
    match(Token::While);
}

///
void Parser::parseFor()
{

}

///
void Parser::parseCall()
{}

///
void Parser::parseNewLines()
{
    while (lookahead.is(Token::NewLine))
        scanner >> lookahead;
}

///
void Parser::match(Token tok)
{
    if (!lookahead.is(tok))
        throw std::string{"Syntax error"};
}

} // basic 
