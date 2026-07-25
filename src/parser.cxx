
#include "parser.hxx"

#include <algorithm>
#include <format>
#include <iostream>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <utility>

using namespace std::string_view_literals;

namespace basic {

class ParseError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};


Parser::Parser(Scanner& sc)
    : scanner{sc}
{
    program = node<Program>(scanner.fileName().string(), 1);
}


Parser::~Parser()
{}


ProgramPtr Parser::parse()
{
    try {
        parseProgram();
    }
    catch( ParseError& e ) {
        std::cerr << "Վերլուծության սխալ։ " << e.what() << std::endl;
        return nullptr;
    }

    return program;
}

//
// Program [NewLines] { Subroutine NewLines }.
//
void Parser::parseProgram()
{
    // կարդալ ամենաառաջին լեքսեմը
    scanner >> lookahead;

    // կարդալ ու դեն նետել ծրագրի սկզբի դատարկ տողերը
    if( lookahead.is(Token::NewLine) )
        parseNewLines();

    // վերլուծել ենթածրագրերի հաջորդականությունը
    while( !lookahead.is(Token::Eof) ) {
        parseSubroutine();
        parseNewLines();
    }

    match(Token::Eof);
}

//
// Subroutine = 'SUB' IDENT ['(' [IdentList] ')'] Statements 'END' 'SUB'.
//
void Parser::parseSubroutine()
{
    // վերնագիր
    auto line = lookahead.line;
    match(Token::Subroutine);
    auto name = match(Token::Identifier);

    // պարամետրերի ցուցակ
    std::vector<std::string> params;
    if( lookahead.is(Token::LeftPar) ) {
        match(Token::LeftPar);
        if( lookahead.is(Token::Identifier) ) {
            auto idlex = match(Token::Identifier);
            params.push_back(idlex);
            while( lookahead.is(Token::Comma) ) {
                match(Token::Comma);
                idlex = match(Token::Identifier);
                params.push_back(idlex);
            }
        }
        match(Token::RightPar);
    }

    auto subr = node<Subroutine>(name, params, line);
    program->members.push_back(subr);

    // մարմին
    subr->body = parseStatements();

    match(Token::End);
    match(Token::Subroutine);
}

//
// Statements = NewLines { Statement NewLines }.
//
StatementPtr Parser::parseStatements()
{
    parseNewLines();

    auto sequ = node<Sequence>(lookahead.line);
    while( lookahead.is(Token::Let, Token::Input, Token::Print, Token::If, Token::While, Token::For, Token::Call) ) {
        auto stat = parseOneStatement();
        sequ->items.push_back(stat);
        parseNewLines();
    }

    return sequ;
}

//
// Statement = Let | Input | Print | If | While | For | Call.
//
StatementPtr Parser::parseOneStatement()
{
    if( lookahead.is(Token::Let) )
        return parseLet();
    
    if( lookahead.is(Token::Input) )
        return parseInput();
    
    if( lookahead.is(Token::Print) )
        return parsePrint();
    
    if( lookahead.is(Token::If) )
        return parseIf();

    if( lookahead.is(Token::While) )
        return parseWhile();

    if( lookahead.is(Token::For) )
        return parseFor();
    
    if( lookahead.is(Token::Call) )
        return parseCall();

    // unreachable
    return {};
}

//
// Let = 'LET' IDENT '=' Expression.
//
StatementPtr Parser::parseLet()
{
    auto ln = lookahead.line;

    match(Token::Let);
    auto vnm = match(Token::Identifier);
    match(Token::Eq);
    auto exo = parseExpression();

    auto varp = node<Variable>(vnm, ln);
    return node<Let>(varp, exo, ln);
}

//
// Input = 'INPUT' IDENT.
//
StatementPtr Parser::parseInput()
{
    auto ln = lookahead.line;
    match(Token::Input);

    std::string prom = "?";
    Position textLine = ln;
    if( lookahead.is(Token::Text) ) {
        textLine = lookahead.line;
        prom = match(Token::Text);
        match(Token::Comma);
    }

    auto vnm = match(Token::Identifier);

    auto varp = node<Variable>(vnm, ln);
    return node<Input>(node<Text>(prom, textLine), varp, ln);
}

//
// Print = 'PRINT' Expression.
//
StatementPtr Parser::parsePrint()
{
    auto ln = lookahead.line;
    match(Token::Print);
    auto exo = parseExpression();
    return node<Print>(exo, ln);
}

//
// If = 'IF' Expression 'THEN' Statements
//   {'ELSEIF' Expression 'THEN' Statements }
//   ['ELSE' Statements] 'END' 'IF'.
//
StatementPtr Parser::parseIf()
{
    auto ifLine = lookahead.line;
    auto empty = node<Statement>(NodeKind::Empty, lookahead.line);

    match(Token::If);
    auto cond = parseExpression();
    match(Token::Then);
    auto deci = parseStatements();
    auto sif = node<If>(cond, deci, empty, ifLine);

    auto it = sif;
    while( lookahead.is(Token::ElseIf) ) {
        auto elifLine = lookahead.line;
        match(Token::ElseIf);
        auto cone = parseExpression();
        match(Token::Then);
        auto dece = parseStatements();
        auto eif = node<If>(cone, dece, empty, elifLine);
        it->alternative = eif;
        it = eif;
    }

    if( lookahead.is(Token::Else) ) {
        match(Token::Else);
        auto alte = parseStatements();
        it->alternative = alte;
    }

    match(Token::End);
    match(Token::If);

    return sif;
}

//
// While = 'WHILE' Expression Statements 'END' 'WHILE'.
//
StatementPtr Parser::parseWhile()
{
    auto ln = lookahead.line;
    match(Token::While);
    auto cond = parseExpression();
    auto body = parseStatements();
    match(Token::End);
    match(Token::While);
    return node<While>(cond, body, ln);
}

//
// For = 'FOR' IDENT '=' Expression 'TO' Expression ['STEP' NUMBER]
//    Statements 'END' 'FOR'.
//
StatementPtr Parser::parseFor()
{
    auto forLine = lookahead.line;
    match(Token::For);
    auto par = match(Token::Identifier);
    match(Token::Eq);
    auto be = parseExpression();
    match(Token::To);
    auto en = parseExpression();
    double spvl = 1;
    Position stepLine = forLine;
    if( lookahead.is(Token::Step) ) {
        match(Token::Step);
        bool neg = false;
        if( lookahead.is(Token::Sub) ) {
            match(Token::Sub);
            neg = true;
        }
        stepLine = lookahead.line;
        auto lex = match(Token::Number);
        spvl = std::stod(lex);
        if( neg )
            spvl = -spvl;
    }
    auto sp = node<Number>(spvl, stepLine);
    auto vp = node<Variable>(par, forLine);
    auto dy = parseStatements();
    match(Token::End);
    match(Token::For);

    return node<For>(vp, be, en, sp, dy, forLine);
}

//
// Call = 'CALL' IDENT [ExpressionList].
//
StatementPtr Parser::parseCall()
{
    auto ln = lookahead.line;
    match(Token::Call);
    auto name = match(Token::Identifier);
    std::vector<ExpressionPtr> args;

    if( lookahead.is(Token::Number, Token::Text, Token::Identifier, 
        Token::Sub, Token::Not, Token::LeftPar) ) {
        auto exo = parseExpression();
        args.push_back(exo);
        while( lookahead.is(Token::Comma) ) {
            match(Token::Comma);
            exo = parseExpression();
            args.push_back(exo);
        }
    }

    return node<Call>(nullptr, args, ln);
}

//
Operation opCode( Token tok )
{
    static std::map<Token, Operation> opcodes{
        { Token::Add, Operation::Add },
        { Token::Sub, Operation::Sub },
        { Token::Amp, Operation::Conc },
        { Token::Mul, Operation::Mul },
        { Token::Div, Operation::Div },
        { Token::Mod, Operation::Mod },
        { Token::Pow, Operation::Pow },
        { Token::Eq, Operation::Eq },
        { Token::Ne, Operation::Ne },
        { Token::Gt, Operation::Gt },
        { Token::Ge, Operation::Ge },
        { Token::Lt, Operation::Lt },
        { Token::Le, Operation::Le },
        { Token::And, Operation::And },
        { Token::Or, Operation::Or }
    };
    return opcodes[tok];
}

//
// Expression = Addition [('=' | '<>' | '>' | '>=' | '<' | '<=') Addition].
//
ExpressionPtr Parser::parseExpression()
{
    auto res = parseAddition();
    if( lookahead.is(Token::Eq, Token::Ne, Token::Gt, Token::Ge, Token::Lt, Token::Le) ) {
        auto opc = opCode(lookahead.kind);
        auto ln = lookahead.line;
        match(lookahead.kind);
        auto exo = parseAddition();
        res = node<Binary>(opc, res, exo, ln);
    }
    return res;
}

//
// Addition = Multiplication {('+' | '-' | '&' | 'OR') Multiplication}.
//
ExpressionPtr Parser::parseAddition()
{
    auto res = parseMultiplication();
    while( lookahead.is(Token::Add, Token::Sub, Token::Amp, Token::Or) ) {
        auto opc = opCode(lookahead.kind);
        auto ln = lookahead.line;
        match(lookahead.kind);
        auto exo = parseMultiplication();
        res = node<Binary>(opc, res, exo, ln);
    }
    return res;
}

//
// Multiplication = Power {('*' | '/' | '\' | 'AND') Power}.
//
ExpressionPtr Parser::parseMultiplication()
{
    auto res = parsePower();
    while( lookahead.is(Token::Mul, Token::Div, Token::Mod, Token::And) ) {
        auto opc = opCode(lookahead.kind);
        auto ln = lookahead.line;
        match(lookahead.kind);
        auto exo = parsePower();
        res = node<Binary>(opc, res, exo, ln);
    }
    return res;
}

//
// Power = Factor ['^' Power].
//
ExpressionPtr Parser::parsePower()
{
    auto res = parseFactor();
    if( lookahead.is(Token::Pow) ) {
        auto ln = lookahead.line;
        match(Token::Pow);
        auto exo = parseFactor();
        res = node<Binary>(Operation::Pow, res, exo, ln);
    }
    return res;
}

//
// Factor = NUMBER | TEXT | IDENT | '(' Expression ')'
//     | IDENT '(' [ExpressionList] ')'.
//
ExpressionPtr Parser::parseFactor()
{
    // TRUE կամ FALSE
    if( lookahead.is(Token::True) ) {
        auto ln = lookahead.line;
        match(Token::True);
        return node<Boolean>(true, ln);
    } 
    else if( lookahead.is(Token::False) ) {
        auto ln = lookahead.line;
        match(Token::False);
        return node<Boolean>(false, ln);
    }

    // NUMBER
    if( lookahead.is(Token::Number) ) {
        auto ln = lookahead.line;
        auto lex = match(Token::Number);
        return node<Number>(std::stod(lex), ln);
    }

    // TEXT
    if( lookahead.is(Token::Text) ) {
        auto ln = lookahead.line;
        auto lex = match(Token::Text);
        return node<Text>(lex, ln);
    }

    // ('-' | 'NOT') Factor
    if( lookahead.is(Token::Sub, Token::Not) ) {
        auto ln = lookahead.line;
        Operation opc = Operation::None;
        if( lookahead.is(Token::Sub) ) {
            opc = Operation::Sub;
            match(Token::Sub);
        }
        else if( lookahead.is(Token::Not) ) {
            opc = Operation::Not;
            match(Token::Not);
        }
        auto exo = parseFactor();
        return node<Unary>(opc, exo, ln);
    }

    // IDENT ['(' [ExpressionList] ')']
    if( lookahead.is(Token::Identifier) ) {
        auto ln = lookahead.line;
        auto name = match(Token::Identifier);
        if( lookahead.is(Token::LeftPar) ) {
            std::vector<ExpressionPtr> args;
            match(Token::LeftPar);
            if( lookahead.is(Token::True, Token::False, Token::Number, 
                             Token::Text, Token::Identifier, Token::Sub, 
                             Token::Not, Token::LeftPar) ) {
                auto exo = parseExpression();
                args.push_back(exo);
                while( lookahead.is(Token::Comma) ) {
                    match(Token::Comma);
                    exo = parseExpression();
                    args.push_back(exo);
                }
            }
            match(Token::RightPar);

            return node<Apply>(nullptr, args, ln);
        }
        return node<Variable>(name, ln);
    }

    // '(' Expression ')'
    if( lookahead.is(Token::LeftPar) ) {
        match(Token::LeftPar);
        auto exo = parseExpression();
        match(Token::RightPar);
        return exo;
    }

    throw ParseError("Սպասվում է NUMBER, TEXT, '-', NOT, IDENT կամ '(', բայց հանդիպել է " + lookahead.value + "։");
}

//
void Parser::parseNewLines()
{
    match(Token::NewLine);
    while( lookahead.is(Token::NewLine) )
        match(Token::NewLine);
}

//
std::string Parser::match(Token exp)
{
    if( !lookahead.is(exp) )
        throw ParseError(std::format("Սպասվում է {}, բայց հանդիպել է {}։", 
                toString(exp), lookahead.value));

    const auto value = lookahead.value;
    scanner >> lookahead;
    return value;
}



} // basic
