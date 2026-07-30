#include "parser.hxx"

#include <algorithm>
#include <exception>
#include <format>
#include <iostream>
#include <memory>
#include <ranges>
#include <set>
#include <utility>

using namespace std::string_view_literals;

namespace basic {

const std::set<Token> FirstStat = {Token::Let, Token::Dim, Token::Input, Token::Print, Token::If, Token::While, Token::For, Token::Call};
const std::set<Token> FirstExpr = {Token::True, Token::False, Token::Number, Token::Text, Token::Identifier, Token::Sub, Token::Not, Token::LeftPar};

//! @brief Թոքենները, որոնցով ավարտվում է հրամանների հաջորդականությունը
const std::set<Token> FollowStat = {Token::End, Token::ElseIf, Token::Else, Token::Subroutine, Token::Eof};

//! @brief Հրամանի մակարդակի համաժամեցման կետերը
const std::set<Token> StatementSync = {
    Token::NewLine, Token::Let, Token::Dim, Token::Input, Token::Print,
    Token::If, Token::While, Token::For, Token::Call,
    Token::End, Token::ElseIf, Token::Else, Token::Subroutine, Token::Eof
};

//! @brief Ենթածրագրի մակարդակի համաժամեցման կետերը
const std::set<Token> SubroutineSync = {Token::Subroutine, Token::Eof};

//! @brief Արտահայտության մակարդակի համաժամեցման կետերը
//!
//! Բացի արտահայտություն սկսող թոքեններից՝ պարունակում է նաև փակող
//! թոքենները, որպեսզի սխալ արտահայտությունը չկլանի իրեն շրջապատող
//! կառուցվածքը։
const std::set<Token> ExprSync = {
    Token::True, Token::False, Token::Number, Token::Text, Token::Identifier,
    Token::Sub, Token::Not, Token::LeftPar,
    Token::RightPar, Token::RightBrack, Token::Comma,
    Token::NewLine, Token::End, Token::ElseIf, Token::Else, Token::Subroutine, Token::Eof
};

Parser::Parser(Scanner& sc)
    : scanner{sc}
{}

Parser::~Parser() = default;

bool Parser::hasErrors() const noexcept
{
    return diagnostics.hasErrors();
}

const std::vector<Diagnostic>& Parser::getErrors() const noexcept
{
    return diagnostics.all();
}

void Parser::advance()
{
    lookahead = scanner.scan();
    ++tokenIndex;
}

void Parser::mark(std::string_view message)
{
    diagnostics.mark(lookahead, message, tokenIndex);
}

//! @brief Համաժամեցման կետ (Վիրտի sync)
//!
//! Եթե ընթացիկ թոքենը սպասվողներից չէ, գրանցվում է սխալը և թոքենները
//! բաց են թողնվում մինչև @c stops բազմության որևէ տարրը։
void Parser::sync(const std::set<Token>& stops, std::string_view message)
{
    if( stops.contains(lookahead.kind) )
        return;

    mark(message);

    while( !stops.contains(lookahead.kind) )
        advance();
}

Program::Ptr Parser::parse()
{
    return parseProgram();
}

// Program = [NewLines] { Subroutine NewLines }.
Program::Ptr Parser::parseProgram()
{
    // առաջին թոքենը
    advance();
    auto line = lookahead.line;

    // [NewLines]. ֆայլի սկզբում դատարկ տողերը պարտադիր չեն
    if( lookahead.is(Token::NewLine) )
        parseNewLines();

    std::vector<Subroutine::Ptr> subroutines;
    while( !lookahead.is(Token::Eof) ) {
        // sync
        sync(SubroutineSync, std::format("Սպասվում է 'SUB', բայց հանդիպել է {}։", describe(lookahead)));

        if( lookahead.is(Token::Subroutine) )
            subroutines.push_back(parseSubroutine());

        parseNewLines();
    }

    return node<Program>(std::move(subroutines), line);
}

// Subroutine = 'SUB' IDENT ['(' [IdentList] ')'] Statements 'END' 'SUB'.
Subroutine::Ptr Parser::parseSubroutine()
{
    auto line = lookahead.line;

    match(Token::Subroutine);
    auto name = match(Token::Identifier); // ենթածրագրի անունը

    // պարամետրերը՝ որպես փոփոխականներ
    std::vector<Variable::Ptr> parameters;
    if( lookahead.is(Token::LeftPar) ) {
        match(Token::LeftPar);
        if( lookahead.is(Token::Identifier) ) {
            auto param = match(Token::Identifier);
            parameters.push_back(node<Variable>(param, line));
            while( lookahead.is(Token::Comma) ) {
                match(Token::Comma);
                param = match(Token::Identifier);
                parameters.push_back(node<Variable>(param, line));
            }
        }
        match(Token::RightPar);
    }

    // մարմինը
    auto body = parseSequence();

    parseBlockEnd(Token::Subroutine);

    return node<Subroutine>(name, parameters, body, line);
}

// Statements = NewLines { Statement NewLines }.
Sequence::Ptr Parser::parseSequence()
{
    auto line = lookahead.line;
    parseNewLines();

    std::vector<Statement::Ptr> items;
    while( !FollowStat.contains(lookahead.kind) ) {
        if( lookahead.is(Token::NewLine) ) {
            parseNewLines();
            continue;
        }

        // sync
        sync(StatementSync, std::format("Սպասվում է հրաման, բայց հանդիպել է {}։", describe(lookahead)));

        if( FirstStat.contains(lookahead.kind) ) {
            items.push_back(parseOneStatement());
            parseNewLines();
        }
    }

    return node<Sequence>(std::move(items), line);
}

// Statement = Let | Dim | Input | Print | If | While | For | Call.
Statement::Ptr Parser::parseOneStatement()
{
    if( lookahead.is(Token::Let) )
        return parseLet();

    if( lookahead.is(Token::Dim) )
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

    // կանչվում է միայն FirstStat-ի թոքենի դեպքում
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
Input::Ptr Parser::parseInput()
{
    auto line = lookahead.line;

    match(Token::Input);
    auto vnm = match(Token::Identifier);

    return node<Input>(node<Variable>(vnm, line), line);
}

// Print = 'PRINT' Expression.
Print::Ptr Parser::parsePrint()
{
    auto line = lookahead.line;

    match(Token::Print);
    auto expr = parseExpression();

    return node<Print>(expr, line);
}

// If = 'IF' Expression 'THEN' Statements {'ELSEIF' Expression 'THEN' Statements } ['ELSE' Statements] 'END' 'IF'.
If::Ptr Parser::parseIf()
{
    auto line = lookahead.line;

    auto first = parseIfThen(true);
    std::vector<If::IfThen::Ptr> branches;
    branches.push_back(first);

    while( lookahead.is(Token::ElseIf) ) {
        auto s = parseIfThen(false);
        branches.push_back(s);
    }

    Statement::Ptr alternative{nullptr};
    if( lookahead.is(Token::Else) ) {
        match(Token::Else);
        alternative = parseSequence();
    }

    parseBlockEnd(Token::If);

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
While::Ptr Parser::parseWhile()
{
    auto line = lookahead.line;

    match(Token::While);
    auto cond = parseExpression();
    auto body = parseSequence();
    parseBlockEnd(Token::While);

    return node<While>(cond, body, line);
}

// For = 'FOR' IDENT '=' Expression 'TO' Expression ['STEP' NUMBER] Statements 'END' 'FOR'.
For::Ptr Parser::parseFor()
{
    auto line = lookahead.line;

    match(Token::For);

    // պարամետրը
    auto name = match(Token::Identifier);
    auto parameter = node<Variable>(name, line);
    match(Token::Eq);
    auto begin = parseExpression();

    // վերջին արժեքը
    match(Token::To);
    auto end = parseExpression();

    // քայլը
    auto step = node<Number>(1.0, line);
    if( lookahead.is(Token::Step) ) {
        match(Token::Step);
        bool neg = false;
        if( lookahead.is(Token::Sub) ) {
            match(Token::Sub);
            neg = true;
        }
        step = parseNumber();
        if( neg )
            step = node<Number>(-step->_value, line);
    }

    // մարմինը
    auto body = parseSequence();

    parseBlockEnd(Token::For);

    return node<For>(parameter, begin, end, step, body, line);
}

// Call = 'CALL' IDENT [ExpressionList].
Call::Ptr Parser::parseCall()
{
    auto line = lookahead.line;

    match(Token::Call);
    auto name = match(Token::Identifier);

    auto arguments = parseExpressionList();
    return node<Call>(name, std::move(arguments), line);
}

std::vector<Expression::Ptr> Parser::parseExpressionList()
{
    if( !FirstExpr.contains(lookahead.kind) )
        return {};

    std::vector<Expression::Ptr> items;
    items.push_back(parseExpression());
    while( lookahead.is(Token::Comma) ) {
        match(Token::Comma);
        items.push_back(parseExpression());
    }
    return items;
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
        { Token::Quot, Operation::Quot },
        { Token::Pow, Operation::Pow },
        { Token::Not, Operation::Not },
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

Expression::Ptr Parser::parsePower()
{
    auto res = parseUnary();
    if( lookahead.is(Token::Pow) ) {
        auto ln = lookahead.line;
        match(Token::Pow);
        auto exo = parsePower();
        res = node<Binary>(Operation::Pow, res, exo, ln);
    }
    return res;
}

Expression::Ptr Parser::parseUnary()
{
    auto line = lookahead.line;

    std::vector<Operation> operations;
	while( lookahead.is(Token::Add, Token::Sub, Token::Not) ) {
		auto op = lookahead.kind;
		match(op);
		operations.insert(operations.begin(), opCode(op));
	}

	auto right = parseSubscript();

    for( auto& op : operations )
        right = node<Unary>(op, right, line);

	return right;
}

Expression::Ptr Parser::parseSubscript()
{
    auto line = lookahead.line;

	auto base = parseFactor();

    while( lookahead.is(Token::LeftBrack) ) {
		match(Token::LeftBrack);
		auto index = parseExpression();
		match(Token::RightBrack);
		base = node<Binary>(Operation::Index, base, index, line);
	}

	return base;
}

Expression::Ptr Parser::parseFactor()
{
    // sync
    if( !FirstExpr.contains(lookahead.kind) ) {
        mark(std::format("Սպասվում է արտահայտություն, բայց հանդիպել է {}։", describe(lookahead)));

        while( !ExprSync.contains(lookahead.kind) )
            advance();
    }

    if( lookahead.is(Token::True, Token::False) )
        return parseTrueOrFalse();

    if( lookahead.is(Token::Number) )
        return parseNumber();

    if( lookahead.is(Token::Text) )
        return parseText();

    if( lookahead.is(Token::Identifier) )
        return parseIdentOrApply();

    if( lookahead.is(Token::LeftPar) )
        return parseGrouped();

    // արտահայտություն այդպես էլ չգտնվեց. վերադարձնում ենք չեզոք հանգույց,
    // որպեսզի կանչողները շարունակեն սովորական ձևով
    return node<Number>(0.0, lookahead.line);
}

Boolean::Ptr Parser::parseTrueOrFalse()
{
    auto line = lookahead.line;
    auto value = match(lookahead.kind);
    return node<Boolean>(value == "TRUE", line);
}

Number::Ptr Parser::parseNumber()
{
    auto line = lookahead.line;
    auto value = match(Token::Number);

    // std::stod-ը դեն է նետում out_of_range՝ չափազանց մեծ հաստատունի դեպքում
    try {
        return node<Number>(value.empty() ? 0.0 : std::stod(value), line);
    }
    catch( const std::exception& ) {
        mark(std::format("Սխալ թվային հաստատուն՝ '{}'։", value));
        return node<Number>(0.0, line);
    }
}

Text::Ptr Parser::parseText()
{
    auto line = lookahead.line;
    auto value = match(Token::Text);
    return node<Text>(value, line);
}

Expression::Ptr Parser::parseArrayDefinition()
{
    match(Token::LeftBrack);
    // TODO
    match(Token::RightBrack);
    return {};
}

Expression::Ptr Parser::parseIdentOrApply()
{
    auto line = lookahead.line;

    // variable
    auto name = match(Token::Identifier);
    if( !lookahead.is(Token::LeftPar) )
        return node<Variable>(name, line);

    // apply
    match(Token::LeftPar);
    auto arguments = parseExpressionList();
    match(Token::RightPar);
    return node<Apply>(name, std::move(arguments), line);
}

Expression::Ptr Parser::parseGrouped()
{
    match(Token::LeftPar);
    auto expr = parseExpression();
    match(Token::RightPar);
    return expr;
}

// NewLines = NL { NL }.
void Parser::parseNewLines()
{
    // ֆայլի ավարտն ինքնին տողի ավարտ է
    if( !lookahead.is(Token::NewLine, Token::Eof) )
        mark(std::format("Սպասվում է տողի ավարտ, բայց հանդիպել է {}։", describe(lookahead)));

    while( lookahead.is(Token::NewLine) )
        advance();
}

std::string Parser::match(Token exp)
{
    if( !lookahead.is(exp) ) {
        mark(std::format("Սպասվում է '{}', բայց հանդիպել է {}։", toString(exp), describe(lookahead)));
        return {};
    }

    const auto value = lookahead.value;
    advance();
    return value;
}

void Parser::parseBlockEnd(Token keyword)
{
    // 'END'-ը բացակայելիս բանալի բառը չենք էլ փնտրում, որպեսզի մեկ սխալի
    // համար երկու հաղորդագրություն չստացվի
    if( !lookahead.is(Token::End) ) {
        mark(std::format("Սպասվում է 'END {}', բայց հանդիպել է {}։", toString(keyword), describe(lookahead)));
        return;
    }

    advance();
    match(keyword);
}

} // basic
