
#include "parser.hxx"

#include <algorithm>
#include <exception>
#include <iostream>

using namespace std::string_view_literals;

namespace basic {
//
class ParseError : public std::exception {
public:
    ParseError(const std::string& mes)
        : message{mes}
    {}

    const char* what() const noexcept override
    {
        return message.c_str();
    }

private:
    std::string message;
};


//
bool equalNames(std::string_view no, std::string_view ni)
{
    std::string_view so = no, si = ni;
    if( '$' == so.back() || '?' == so.back() )
        so.remove_suffix(1);
    if( '$' == si.back() || '?' == si.back() )
        si.remove_suffix(1);
    return so == si;
}


///
Parser::Parser(const std::filesystem::path& filename)
    : scanner{filename}
{
    builtins = {
        // թվային ֆունկցիաներ
        BuiltIn{"SQR", {"a"}, true},
        BuiltIn{"SIN", {"a"}, true},

        // տեքստային ֆունկցիաներ
        BuiltIn{"MID$", {"a$", "b", "c"}, true},
        BuiltIn{"STR$", {"a"}, true}
    };

    module = node<Program>(filename.string());
}

///
Parser::~Parser()
{}

///
ProgramPtr Parser::parse()
{
    try {
        parseProgram();
    }
    catch( ParseError& e ) {
        std::cerr << "Վերլուծության սխալ։ " << e.what() << std::endl;
        return nullptr;
    }

    if( unresolved.empty() )
        return module;

    // տուգել, որ unresolved ցուցակը դատարկ լինի, այսինքն՝
    // ծրագրում ոչ մի տեղ չսահմանված ֆունկցիայի հղում չմնա
    for( auto& e : unresolved ) {
        std::string mes = e.first + " անունով ենթածրագիրը սահմանված չէ։";
        std::cerr << "Վերլուծության սխալ։ " << mes << std::endl;
        // TODO: նշել կանչերի տեղերը
    }

    return nullptr;
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
    match(Token::Subroutine);
    auto name = lookahead.value;
    match(Token::Identifier);

    // ստուգել name անունով ենթածրագրի արդեն հայտարարված լինելը,
    // ուշադրություն դարձնել անվան վերջի '$' նիշին (f և f$ անունները նույնն են)
    auto sbit = find_if(module->members.begin(), module->members.end(),
        [&name](auto sp) -> bool { return equalNames(name, sp->name); });
    if (sbit != module->members.end())
        throw ParseError(name + " անունով ենթածրագիրն արդեն սահմանված է։");

    // պարամետրերի ցուցակ
    std::vector<std::string> params;
    if( lookahead.is(Token::LeftPar) ) {
        match(Token::LeftPar);
        if( lookahead.is(Token::Identifier) ) {
            auto idlex = lookahead.value;
            match(Token::Identifier);
            params.push_back(idlex);
            while( lookahead.is(Token::Comma) ) {
                match(Token::Comma);
                idlex = lookahead.value;
                match(Token::Identifier);
                params.push_back(idlex);
            }
        }
        match(Token::RightPar);
    }

    currentsubr = node<Subroutine>(name, params);
    module->members.push_back(currentsubr);

	// պարամետրերն ավելացնել ենթածրագրի լոկալ անունների ցուցակում
	for( auto& ps : currentsubr->parameters )
        currentsubr->locals.push_back(node<Variable>(ps));

    // մարմին
    currentsubr->body = parseStatements();

    match(Token::End);
    match(Token::Subroutine);

    // անորոշ հղումների ցուցակում ճշտել, թե որ Apply օբյեկտներն են
    // հղվում այս ենթածրագրին, և ուղղել/լրացնել պակասող տվյալները
    auto apit = unresolved.find(name);
    if( apit != unresolved.end() ) {
        for( auto& ap : apit->second )
            ap->callee = currentsubr;
        unresolved.erase(apit);
    }
}

//
// Statements = NewLines { (Let | Input | Print | If | While | For | Call) NewLines }.
//
StatementPtr Parser::parseStatements()
{
    parseNewLines();

    auto sequ = node<Sequence>();
    while( true ) {
        StatementPtr stat;
        unsigned int line = lookahead.line;
        if( lookahead.is(Token::Let) )
            stat = parseLet();
        else if( lookahead.is(Token::Input) )
            stat = parseInput();
        else if( lookahead.is(Token::Print) )
            stat = parsePrint();
        else if( lookahead.is(Token::If) )
            stat = parseIf();
        else if( lookahead.is(Token::While) )
            stat = parseWhile();
        else if( lookahead.is(Token::For) )
            stat = parseFor();
        else if( lookahead.is(Token::Call) )
            stat = parseCall();
        else
            break;
        stat->line = line;
        sequ->items.push_back(stat);
        parseNewLines();
    }

    return sequ;
}

//
// Let = 'LET' IDENT '=' Expression.
//
StatementPtr Parser::parseLet()
{
    unsigned int pos = lookahead.line;

    match(Token::Let);
    auto vnm = lookahead.value;
    match(Token::Identifier);
    match(Token::Eq);
    auto exo = parseExpression();

    auto varp = getVariable(vnm, false);

    // եթե vnm-ն համընկնում է ընթացիկ ենթածրագրի անվան հետ,
    // ապա վերջինիս hasValue-ն դնել true
    if( vnm == currentsubr->name )
        currentsubr->hasValue = true;

    return node<Let>(varp, exo);
}

//
// Input = 'INPUT' IDENT.
//
StatementPtr Parser::parseInput()
{
    match(Token::Input);

    std::string prom = "?";
    if( lookahead.is(Token::Text) ) {
        prom = lookahead.value;
        match(Token::Text);
        match(Token::Comma);
    }

    auto vnm = lookahead.value;
    match(Token::Identifier);

    auto varp = getVariable(vnm, false);
    return node<Input>(node<Text>(prom), varp);
}

//
// Print = 'PRINT' Expression.
//
StatementPtr Parser::parsePrint()
{
    match(Token::Print);
    auto exo = parseExpression();
    return node<Print>(exo);
}

//
// If = 'IF' Expression 'THEN' Statements
//   {'ELSEIF' Expression 'THEN' Statements }
//   ['ELSE' Statements] 'END' 'IF'.
//
StatementPtr Parser::parseIf()
{
    match(Token::If);
    auto cond = parseExpression();
    match(Token::Then);
    auto deci = parseStatements();
    auto sif = node<If>(cond, deci);

    auto it = sif;
    while( lookahead.is(Token::ElseIf) ) {
        match(Token::ElseIf);
        auto cone = parseExpression();
        match(Token::Then);
        auto dece = parseStatements();
        auto eif = node<If>(cone, dece);
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
    match(Token::While);
    auto cond = parseExpression();
    auto body = parseStatements();
    match(Token::End);
    match(Token::While);
    return node<While>(cond, body);
}

//
// For = 'FOR' IDENT '=' Expression 'TO' Expression ['STEP' NUMBER]
//    Statements 'END' 'FOR'.
//
StatementPtr Parser::parseFor()
{
    match(Token::For);
    auto par = lookahead.value;
    match(Token::Identifier);
    match(Token::Eq);
    auto be = parseExpression();
    match(Token::To);
    auto en = parseExpression();
    double spvl = 1;
    if( lookahead.is(Token::Step) ) {
        match(Token::Step);
        bool neg = false;
        if( lookahead.is(Token::Sub) ) {
            match(Token::Sub);
            neg = true;
        }
        auto lex = lookahead.value;
        match(Token::Number);
        spvl = std::stod(lex);
        if( neg )
            spvl = -spvl;
    }
    auto sp = node<Number>(spvl);
    auto vp = getVariable(par, false);
    auto dy = parseStatements();
    match(Token::End);
    match(Token::For);

    return node<For>(vp, be, en, sp, dy);
}

//
// Call = 'CALL' IDENT [ExpressionList].
//
StatementPtr Parser::parseCall()
{
    match(Token::Call);
    auto name = lookahead.value;
    match(Token::Identifier);
    std::vector<ExpressionPtr> args;

    if( lookahead.is({ Token::Number, Token::Text, Token::Identifier, 
        Token::Sub, Token::Not, Token::LeftPar }) ) {
        auto exo = parseExpression();
        args.push_back(exo);
        while( lookahead.is(Token::Comma) ) {
            match(Token::Comma);
            exo = parseExpression();
            args.push_back(exo);
        }
    }

    auto caller = node<Call>(nullptr, args);

    auto callee = getSubroutine(name);
    if( nullptr == callee )
        unresolved[name].push_back(caller->subrCall);

    caller->subrCall->callee = callee;

    return caller;
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
    if( lookahead.is({ Token::Eq, Token::Ne, Token::Gt, Token::Ge, Token::Lt, Token::Le }) ) {
        auto opc = opCode(lookahead.kind);
        match(lookahead.kind);
        auto exo = parseAddition();
        res = node<Binary>(opc, res, exo);
    }
    return res;
}

//
// Addition = Multiplication {('+' | '-' | '&' | 'OR') Multiplication}.
//
ExpressionPtr Parser::parseAddition()
{
    auto res = parseMultiplication();
    while( lookahead.is({ Token::Add, Token::Sub, Token::Amp, Token::Or }) ) {
        auto opc = opCode(lookahead.kind);
        match(lookahead.kind);
        auto exo = parseMultiplication();
        res = node<Binary>(opc, res, exo);
    }
    return res;
}

//
// Multiplication = Power {('*' | '/' | '\' | 'AND') Power}.
//
ExpressionPtr Parser::parseMultiplication()
{
    auto res = parsePower();
    while( lookahead.is({ Token::Mul, Token::Div, Token::Mod, Token::And }) ) {
        auto opc = opCode(lookahead.kind);
        match(lookahead.kind);
        auto exo = parsePower();
        res = node<Binary>(opc, res, exo);
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
        match(Token::Pow);
        auto exo = parseFactor();
        res = node<Binary>(Operation::Pow, res, exo);
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
        match(Token::True);
        return node<Boolean>(true);
    } 
    else if( lookahead.is(Token::False) ) {
        match(Token::False);
        return node<Boolean>(false);
    }

    // NUMBER
    if( lookahead.is(Token::Number) ) {
        auto lex = lookahead.value;
        match(Token::Number);
        return node<Number>(std::stod(lex));
    }

    // TEXT
    if( lookahead.is(Token::Text) ) {
        auto lex = lookahead.value;
        match(Token::Text);
        return node<Text>(lex);
    }

    // ('-' | 'NOT') Factor
    if( lookahead.is({ Token::Sub, Token::Not }) ) {
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
        return node<Unary>(opc, exo);
    }

    // IDENT ['(' [ExpressionList] ')']
    if( lookahead.is(Token::Identifier) ) {
        auto name = lookahead.value;
        match(Token::Identifier);
        if( lookahead.is(Token::LeftPar) ) {
            std::vector<ExpressionPtr> args;
            match(Token::LeftPar);
            if( lookahead.is({ Token::True, Token::False, Token::Number, 
                               Token::Text, Token::Identifier, Token::Sub, 
                               Token::Not, Token::LeftPar }) ) {
                auto exo = parseExpression();
                args.push_back(exo);
                while( lookahead.is(Token::Comma) ) {
                    match(Token::Comma);
                    exo = parseExpression();
                    args.push_back(exo);
                }
            }
            match(Token::RightPar);

            auto applyer = node<Apply>(nullptr, args);
            applyer->type = typeOf(name);

            auto callee = getSubroutine(name);
            if( nullptr == callee )
                unresolved[name].push_back(applyer);

            applyer->callee = callee;

            return applyer;
        }
        // ստուգել, որ name անունով փոփոխական սահմանված լինի
        return getVariable(name, true);
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
void Parser::match(Token exp)
{
    if( !lookahead.is(exp) )
        throw ParseError("Սպասվում է " + toString(exp) + 
                ", բայց հանդիպել է " + lookahead.value + "։");

    scanner >> lookahead;
}

//
VariablePtr Parser::getVariable(std::string_view name, bool rval)
{
    auto& locals = currentsubr->locals;

    if( rval && equalNames(currentsubr->name, name) )
        throw ParseError("Ենթածրագրի անունը օգտագործված է որպես փոփոխական։");

    auto vpi = std::find_if(locals.begin(), locals.end(),
        [&name](auto vp) -> bool { return equalNames(name, vp->name); });
    if( locals.end() != vpi )
        return *vpi;

    if( rval )
        throw ParseError(std::string{name} + " փոփոխականը դեռ սահմանված չէ։");

    auto varp = node<Variable>(name); // TODO: review this
    locals.push_back(varp);

    return varp;
}

///
SubroutinePtr Parser::getSubroutine(std::string_view name)
{
    // որոնել տրված անունով ենթածրագիրը արդեն սահմանվածների մեջ
    for( auto si : module->members )
        if( equalNames(si->name, name) )
            return si;

    // որոնել 
    for( auto& bi : builtins )
        if( std::get<0>(bi) == name ) {
            // հայտարարել ներդրված ենթածրագիր
            auto sre = node<Subroutine>(std::get<0>(bi), std::get<1>(bi));
            sre->isBuiltIn = true;
            sre->hasValue = std::get<2>(bi);
            module->members.push_back(sre);
            return sre;        
        }

    return nullptr;

// TODO: այս ստուգումը տեղափոխել TypeChecker
//    // եթե հարցումը ֆունկցիայի կանչի համար է, բայց ենթածրագիրն արժեք չի վերադարձնում
//    if( func && !(*subrit)->hasValue )
//        throw ParseError(nm + " պրոցեդուրան արժեք չի վերադարձնում։");
}

} // basic
