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
{}


Parser::~Parser() = default;


Program::Ptr Parser::parse()
try {
    return parseProgram();
}
catch( ParseError& e ) {
    std::cerr << "Վերլուծության սխալ։ " << e.what() << std::endl;
    return nullptr;
}

// Program = [NewLines] { Subroutine NewLines }.
Program::Ptr Parser::parseProgram()
{
    // առաջին թոքենը
    lookahead = scanner.scan();
    auto line = lookahead.line;

    if( lookahead.is(Token::NewLine) )
        parseNewLines();

    std::vector<Subroutine::Ptr> subroutines;
    while( !lookahead.is(Token::Eof) ) {
        auto s = parseSubroutine();
        parseNewLines();
        subroutines.push_back(s);
    }

    match(Token::Eof);

    return node<Program>(std::move(subroutines), line);
}

// Subroutine = 'SUB' IDENT ['(' [IdentList] ')'] Statements 'END' 'SUB'.
Subroutine::Ptr Parser::parseSubroutine()
{
    auto line = lookahead.line;
    auto name = match(Token::Subroutine);

    std::vector<Variable::Ptr> parameters;
    if( lookahead.is(Token::LeftPar) ) {
        match(Token::LeftPar);
        if( lookahead.is(Token::Identifier) ) {
            auto line = lookahead.line;
            auto idlex = match(Token::Identifier);
            parameters.push_back(node<Variable>(idlex, line));
            while( lookahead.is(Token::Comma) ) {
                match(Token::Comma);
                line = lookahead.line;
                idlex = match(Token::Identifier);
                parameters.push_back(node<Variable>(idlex, line));
            }
        }
        match(Token::RightPar);
    }

    auto body = parseSequence();

    match(Token::End);
    match(Token::Subroutine);

    return node<Subroutine>(name, parameters, body, line);
}

// Statements = NewLines { Statement NewLines }.
Sequence::Ptr Parser::parseSequence()
{
    auto line = lookahead.line;
    parseNewLines();

    std::vector<Statement::Ptr> items;
    while( lookahead.is(Token::Let, Token::Input, Token::Print, Token::If, Token::While, Token::For, Token::Call) ) {
        items.push_back(parseOneStatement());
        parseNewLines();
    }

    return node<Sequence>(std::move(items), line);
}

// Statement = Let | Dim | Input | Print | If | While | For | Call.
Statement::Ptr Parser::parseOneStatement()
{
    if( lookahead.is(Token::Let) )
        return parseLet();

    if (lookahead.is(Token::Dim))
        return parseDim();

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

    return {};
}

// Let = 'LET' IDENT '=' Expression.
Let::Ptr Parser::parseLet()
{
    auto ln = lookahead.line;

    match(Token::Let);
    auto vnm = match(Token::Identifier);
    match(Token::Eq);
    auto exo = parseExpression();

    return node<Let>(node<Variable>(vnm, ln), exo, ln);
}

// Dim = 'DIM' IDENT '[' Expression ']'.
Dim::Ptr Parser::parseDim()
{
    auto line = lookahead.line;

    match(Token::Dim);
    auto name = match(Token::Identifier);
    match(Token::LeftBrack);
    auto size = parseExpression();
    match(Token::RightBrack);

    return node<Dim>(name, size, line);
}

// Input = 'INPUT' IDENT.
Statement::Ptr Parser::parseInput()
{
    auto line = lookahead.line;

    match(Token::Input);
    auto vnm = match(Token::Identifier);

    return node<Input>(node<Variable>(vnm, line), line);
}

// Print = 'PRINT' Expression.
Statement::Ptr Parser::parsePrint()
{
    auto line = lookahead.line;

    match(Token::Print);
    auto expr = parseExpression();

    return node<Print>(expr, line);
}

// If = 'IF' Expression 'THEN' Statements {'ELSEIF' Expression 'THEN' Statements } ['ELSE' Statements] 'END' 'IF'.
Statement::Ptr Parser::parseIf()
{
    auto line = lookahead.line;

    auto first = parseIfThen(true);
    std::vector<If::IfThen::Ptr> branches;
    branches.push_back(first);

    while( lookahead.is(Token::ElseIf) ) {
        auto s = parseIfThen(false);
        branches.push_back(s);
    }

    Statement::Ptr alternative{ nullptr };
    if( lookahead.is(Token::Else) ) {
        match(Token::Else);
        alternative = parseSequence();
    }

    match(Token::End);
    match(Token::If);

    return node<If>(branches, alternative, line);
}

If::IfThen::Ptr Parser::parseIfThen(bool first)
{
    auto line = lookahead.line;

    match(first ? Token::If : Token::ElseIf);
    auto condition = parseExpression();
    match(Token::Then);
    auto decision = parseSequence();

    return node<If::IfThen>(condition, decision, line);
}

// While = 'WHILE' Expression Statements 'END' 'WHILE'.
Statement::Ptr Parser::parseWhile()
{
    auto line = lookahead.line;

    match(Token::While);
    auto cond = parseExpression();
    auto body = parseSequence();
    match(Token::End);
    match(Token::While);

    return node<While>(cond, body, line);
}

//
// For = 'FOR' IDENT '=' Expression 'TO' Expression ['STEP' NUMBER]
//    Statements 'END' 'FOR'.
//
For::Ptr Parser::parseFor()
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
    auto vp = node<Variable>(par, forLine);
    auto dy = parseSequence();
    match(Token::End);
    match(Token::For);

    return node<For>(vp, be, en, node<Number>(spvl, stepLine), dy, forLine);
}

//
// Call = 'CALL' IDENT [ExpressionList].
//
Call::Ptr Parser::parseCall()
{
    auto ln = lookahead.line;
    match(Token::Call);
    auto name = match(Token::Identifier);
    std::vector<Expression::Ptr> args;

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

    return node<Call>(name, std::move(args), ln);
}

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

Expression::Ptr parseExpression()
{
    auto left = parseConjunction();
    while( lookahead.is(Token::Or) ) {
        auto line = lookahead.line;
        match(Token::Or);
        auto right = parseConjunction();
        left = node<Binary>(Operation::Or, left, right, line);
    }

    return left;
}

Expression::Ptr parseExpression()
{
    auto left = parseEquality();
    while (lookahead.is(Token::And)) {
        auto line = lookahead.line;
        match(Token::And);
        auto right = parseEquality();
        left = node<Binary>(Operation::And, left, right, line);
    }

    return left;
}

/*
      Addition = Addition [('=' | '<>' | '<' | '<=' | '>' | '>=') Addition].
      Addition = Multiplication { ('+' | '-' | 'OR' | '&') Multiplication }.
Multiplication = Power { ('*' | '/' | '\' | 'MOD' | 'AND') Power }.
         Power = Unary ['^' Power].
         Unary = { '+' | '-' | 'NOT' } Subscript.
     Subscript = Factor { '[' Expression ']' }.
        Factor = TRUE | FALSE | NUMBER | TEXT | IdentOrApply | Grouping.
ExpressionList = Expression { ',' Expression }.
*/

// Expression = Addition [('=' | '<>' | '>' | '>=' | '<' | '<=') Addition].
Expression::Ptr Parser::parseExpression()
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

// Addition = Multiplication {('+' | '-' | '&' | 'OR') Multiplication}.
Expression::Ptr Parser::parseAddition()
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

// Multiplication = Power {('*' | '/' | '\' | 'MOD' | 'AND') Power}.
Expression::Ptr Parser::parseMultiplication()
{
    auto res = parsePower();
    while( lookahead.is(Token::Mul, Token::Div, Token::Mod, Token::Quot, Token::And) ) {
        auto opc = opCode(lookahead.kind);
        auto ln = lookahead.line;
        match(lookahead.kind);
        auto exo = parsePower();
        res = node<Binary>(opc, res, exo, ln);
    }
    return res;
}

// Power = Factor ['^' Power].
Expression::Ptr Parser::parsePower()
{
    auto res = parseFactor();
    if( lookahead.is(Token::Pow) ) {
        auto ln = lookahead.line;
        match(Token::Pow);
        auto exo = parsePower();
        res = node<Binary>(Operation::Pow, res, exo, ln);
    }
    return res;
}

//
// Factor = NUMBER | TEXT | IDENT | '(' Expression ')'
//     | IDENT '(' [ExpressionList] ')'.
//
Expression::Ptr Parser::parseFactor()
{
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

    if( lookahead.is(Token::Number) ) {
        auto ln = lookahead.line;
        auto lex = match(Token::Number);
        return node<Number>(std::stod(lex), ln);
    }

    if( lookahead.is(Token::Text) ) {
        auto ln = lookahead.line;
        auto lex = match(Token::Text);
        return node<Text>(lex, ln);
    }

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
        return node<Unary>(opc, parseFactor(), ln);
    }

    if( lookahead.is(Token::Identifier) ) {
        auto ln = lookahead.line;
        auto name = match(Token::Identifier);
        if( lookahead.is(Token::LeftPar) ) {
            std::vector<Expression::Ptr> args;
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
            return node<Apply>(name, std::move(args), ln);
        }
        return node<Variable>(name, ln);
    }

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

    const auto val = lookahead.value;
    lookahead = scanner.scan();
    return val;
}

} // basic
