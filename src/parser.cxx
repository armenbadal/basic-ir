
#include "parser.hxx"

#include <algorithm>
#include <exception>
#include <iostream>

namespace basic {
///
Parser::Parser(const std::string& filename)
    : scanner{ filename }
{
    module = new Program(filename);

    // թվային ֆունկցիաներ
    declareBuiltIn("SQR", { "a" }, true);
    declareBuiltIn("SIN", { "a" }, true);

    // տեքստային ֆունկցիաներ
    declareBuiltIn("MID$", { "a", "b", "c$" }, true);
    declareBuiltIn("STR$", { "a" }, true);
}

///
Parser::~Parser()
{
    AstNode::deleteAllocatedNodes();
}

///
Program* Parser::parse()
{
    parseProgram();

    // տուգել, որ unresolved ցուցակը դատարկ լինի, այսինքն՝
    // ծրագրում ոչ մի տեղ չսահմանված ֆունկցիայի հղում չմնա
    for (auto& e : unresolved) {
        std::string mes = e.first + " անունով ենթածրագիրը սահմանված չէ։";
        // TODO: նշել կանչերի տեղերը
        throw ParseError(mes);
    }

    return module;
}

//
// Program [NewLines] { Subroutine NewLines }.
//
void Parser::parseProgram()
{
    // կարդալ ամենաառաջին լեքսեմը
    scanner >> lookahead;

    // կարդալ ու դեն նետել ծրագրի սկզբի դատարկ տողերը
    if (lookahead.is(Token::NewLine))
        parseNewLines();

    // վերլուծել ենթածրագրերի հաջորդականությունը
    while (!lookahead.is(Token::Eof)) {
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
        throw ParseError{ name + " անունով ենթածրագիրն արդեն սահմանված է։" };

    // պարամետրերի ցուցակ
    std::vector<std::string> params;
    if (lookahead.is(Token::LeftPar)) {
        match(Token::LeftPar);
        if (lookahead.is(Token::Identifier)) {
            auto idlex = lookahead.value;
            match(Token::Identifier);
            params.push_back(idlex);
            while (lookahead.is(Token::Comma)) {
                match(Token::Comma);
                idlex = lookahead.value;
                match(Token::Identifier);
                params.push_back(idlex);
            }
        }
        match(Token::RightPar);
    }

    auto subr = new Subroutine(name, params);
    module->members.push_back(subr);

    // մարմին
    subr->body = parseStatements();

    match(Token::End);
    match(Token::Subroutine);

    // անորոշ հղումների ցուցակում ճշտել, թե որ Apply օբյեկտներն են
    // հղվում այս ենթածրագրին, և ուղղել/լրացնել պակասող տվյալները
    auto apit = unresolved.find(name);
    if (apit != unresolved.end()) {
        for (Apply* ap : apit->second)
            ap->procptr = subr;
        unresolved.erase(apit);
    }
}

//
// Statements = NewLines { (Let | Input | Print | If | While | For | Call) NewLines }.
//
Statement* Parser::parseStatements()
{
    parseNewLines();

    auto sequ = new Sequence();
    while (true) {
        Statement* stat = nullptr;
        unsigned int line = lookahead.line;
        if (lookahead.is(Token::Let))
            stat = parseLet();
        else if (lookahead.is(Token::Input))
            stat = parseInput();
        else if (lookahead.is(Token::Print))
            stat = parsePrint();
        else if (lookahead.is(Token::If))
            stat = parseIf();
        else if (lookahead.is(Token::While))
            stat = parseWhile();
        else if (lookahead.is(Token::For))
            stat = parseFor();
        else if (lookahead.is(Token::Call))
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
Statement* Parser::parseLet()
{
    unsigned int pos = lookahead.line;

    match(Token::Let);
    auto vnm = lookahead.value;
    match(Token::Identifier);
    match(Token::Eq);
    auto exo = parseExpression();

    auto varp = getVariable(vnm, false);

    if (varp->type != exo->type)
        throw TypeError("Տիպերի անհամապատասխանություն " + std::to_string(pos) + " տողում։");

    // եթե vnm-ն համընկնում է ընթացիկ ենթածրագրի անվան հետ,
    // ապա վերջինիս hasValue-ն դնել true
    Subroutine* current = module->members.back();
    if (vnm == current->name)
        current->hasValue = true;

    return new Let(varp, exo);
}

//
// Input = 'INPUT' IDENT.
//
Statement* Parser::parseInput()
{
    match(Token::Input);
    std::string prom = "";
    if (lookahead.is(Token::Text)) {
        prom = lookahead.value;
        match(Token::Text);
        match(Token::Comma);
    }
    auto vnm = lookahead.value;
    match(Token::Identifier);

    auto varp = getVariable(vnm, false);
    return new Input(prom, varp);
}

//
// Print = 'PRINT' Expression.
//
Statement* Parser::parsePrint()
{
    match(Token::Print);
    auto exo = parseExpression();
    return new Print(exo);
}

//
// If = 'IF' Expression 'THEN' Statements
//   {'ELSEIF' Expression 'THEN' Statements }
//   ['ELSE' Statements] 'END' 'IF'.
//
Statement* Parser::parseIf()
{
    match(Token::If);
    auto cond = parseExpression();
    match(Token::Then);
    auto deci = parseStatements();
    auto sif = new If(cond, deci);

    auto it = sif;
    while (lookahead.is(Token::ElseIf)) {
        match(Token::ElseIf);
        auto cone = parseExpression();
        match(Token::Then);
        auto dece = parseStatements();
        auto eif = new If(cone, dece);
        it->alternative = eif;
        it = eif;
    }

    if (lookahead.is(Token::Else)) {
        match(Token::Else);
        auto alte = parseStatements();
        it->alternative = alte;
    }

    // TODO: եթե ELSE ճյուղը բացակայում է, ապա կարելի է կամ
    // alternative-ին վերագրել դատարկ Sequence, կամ թողնել
    // nullptr և գեներատորներում ստուգել

    match(Token::End);
    match(Token::If);

    return sif;
}

//
// While = 'WHILE' Expression Statements 'END' 'WHILE'.
//
Statement* Parser::parseWhile()
{
    match(Token::While);
    auto cond = parseExpression();
    auto body = parseStatements();
    match(Token::End);
    match(Token::While);
    return new While(cond, body);
}

//
// For = 'FOR' IDENT '=' Expression 'TO' Expression ['STEP' NUMBER]
//    Statements 'END' 'FOR'.
//
Statement* Parser::parseFor()
{
    match(Token::For);
    auto par = lookahead.value;
    match(Token::Identifier);
    match(Token::Eq);
    auto be = parseExpression();
    match(Token::To);
    auto en = parseExpression();
    Expression* sp = nullptr;
    if (lookahead.is(Token::Step)) {
        match(Token::Step);
        bool neg = false;
        if (lookahead.is(Token::Sub)) {
            match(Token::Sub);
            neg = true;
        }
        auto lex = lookahead.value;
        match(Token::Number);
        sp = new Number(std::stod(lex));
        if (neg)
            sp = new Unary(Operation::Sub, sp);
    }
    else
        sp = new Number(1);
    auto vp = getVariable(par, false);
    auto dy = parseStatements();
    match(Token::End);
    match(Token::For);

    return new For(vp, be, en, sp, dy);
}

//
// Call = 'CALL' IDENT [ExpressionList].
//
Statement* Parser::parseCall()
{
    match(Token::Call);
    auto name = lookahead.value;
    match(Token::Identifier);
    std::vector<Expression*> args;
    if (lookahead.is({ Token::Number, Token::Text, Token::Identifier, 
        Token::Sub, Token::Not, Token::LeftPar })) {
        auto exo = parseExpression();
        args.push_back(exo);
        while (lookahead.is(Token::Comma)) {
            match(Token::Comma);
            exo = parseExpression();
            args.push_back(exo);
        }
    }

    Call* caller = new Call(nullptr, args);

    auto callee = getSubroutine(name, args, false);
    if (nullptr == callee)
        unresolved[name].push_back(caller->subrcall);

    caller->subrcall->procptr = callee;

    return caller;
}

//
Operation opCode(Token tok)
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
Expression* Parser::parseExpression()
{
    auto res = parseAddition();
    if (lookahead.is({ Token::Eq, Token::Ne, Token::Gt, Token::Ge, Token::Lt, Token::Le })) {
        auto opc = opCode(lookahead.kind);
        match(lookahead.kind);
        auto exo = parseAddition();
        res = new Binary(opc, res, exo);
        checkTypes(dynamic_cast<Binary*>(res));
    }
    return res;
}

//
// Addition = Multiplication {('+' | '-' | '&' | 'OR') Multiplication}.
//
Expression* Parser::parseAddition()
{
    auto res = parseMultiplication();
    while (lookahead.is({ Token::Add, Token::Sub, Token::Amp, Token::Or })) {
        auto opc = opCode(lookahead.kind);
        match(lookahead.kind);
        auto exo = parseMultiplication();
        res = new Binary(opc, res, exo);
        checkTypes(dynamic_cast<Binary*>(res));
    }
    return res;
}

//
// Multiplication = Power {('*' | '/' | '\' | 'AND') Power}.
//
Expression* Parser::parseMultiplication()
{
    auto res = parsePower();
    while (lookahead.is({ Token::Mul, Token::Div, Token::Mod, Token::And })) {
        auto opc = opCode(lookahead.kind);
        match(lookahead.kind);
        auto exo = parsePower();
        res = new Binary(opc, res, exo);
        checkTypes(dynamic_cast<Binary*>(res));
    }
    return res;
}

//
// Power = Factor ['^' Power].
//
Expression* Parser::parsePower()
{
    auto res = parseFactor();
    if (lookahead.is(Token::Pow)) {
        match(Token::Pow);
        auto exo = parseFactor();
        res = new Binary(Operation::Pow, res, exo);
        checkTypes(dynamic_cast<Binary*>(res));
    }
    return res;
}

//
// Factor = NUMBER | TEXT | IDENT | '(' Expression ')'
//     | IDENT '(' [ExpressionList] ')'.
//
Expression* Parser::parseFactor()
{
    // NUMBER
    if (lookahead.is(Token::Number)) {
        auto lex = lookahead.value;
        match(Token::Number);
        return new Number(std::stod(lex));
    }

    // TEXT
    if (lookahead.is(Token::Text)) {
        auto lex = lookahead.value;
        match(Token::Text);
        return new Text(lex);
    }

    // ('-' | 'NOT') Factor
    if (lookahead.is({ Token::Sub, Token::Not })) {
        Operation opc = Operation::None;
        if (lookahead.is(Token::Sub)) {
            opc = Operation::Sub;
            match(Token::Sub);
        }
        else if (lookahead.is(Token::Not)) {
            opc = Operation::Not;
            match(Token::Not);
        }
        auto exo = parseFactor();
        if (exo->type != Type::Number)
            throw TypeError("Բացասումն ու ժխտումը կիրառելի է միայն թվերին։");

        return new Unary(opc, exo);
    }

    // IDENT ['(' [ExpressionList] ')']
    if (lookahead.is(Token::Identifier)) {
        auto name = lookahead.value;
        match(Token::Identifier);
        if (lookahead.is(Token::LeftPar)) {
            std::vector<Expression*> args;
            match(Token::LeftPar);
            auto exo = parseExpression();
            args.push_back(exo);
            while (lookahead.is(Token::Comma)) {
                match(Token::Comma);
                exo = parseExpression();
                args.push_back(exo);
            }
            match(Token::RightPar);

            Apply* applyer = new Apply(nullptr, args);
            applyer->type = typeOf(name);

            auto callee = getSubroutine(name, args, true);
            if (nullptr == callee)
                unresolved[name].push_back(applyer);

            applyer->procptr = callee;

            return applyer;
        }
        // ստուգել, որ name անունով փոփոխական սահմանված լինի
        return getVariable(name, true);
    }

    // '(' Expression ')'
    if (lookahead.is(Token::LeftPar)) {
        match(Token::LeftPar);
        auto exo = parseExpression();
        match(Token::RightPar);
        return exo;
    }

    throw ParseError("Սպասվում է NUMBER, TEXT, '-', NOT, IDENT կամ '(', բայց հանդիպել է " + lookahead.value + "։");;
}

//
void Parser::parseNewLines()
{
    match(Token::NewLine);
    while (lookahead.is(Token::NewLine))
        match(Token::NewLine);
}

//
void Parser::match(Token exp)
{
    if (!lookahead.is(exp))
        throw ParseError("Սպասվում է " + toString(exp) + ", բայց հանդիպել է " + lookahead.value + "։");

    scanner >> lookahead;
}

//
void Parser::declareBuiltIn(const std::string& nm, const std::vector<std::string>& ps, bool rv)
{
    Subroutine* sre = new Subroutine(nm, ps);
    sre->isBuiltIn = true;
    sre->hasValue = rv;
    module->members.push_back(sre);
}

//
Variable* Parser::getVariable(const std::string& nm, bool rval)
{
    Subroutine* subr = module->members.back();
    auto& locals = subr->locals;

    if (rval && equalNames(subr->name, nm))
        throw ParseError("Ենթածրագրի անունը օգտագործված է որպես փոփոխական։");

    auto vpi = std::find_if(locals.begin(), locals.end(),
        [&nm](auto vp) -> bool { return equalNames(nm, vp->name); });
    if (locals.end() != vpi)
        return *vpi;

    if (rval)
        throw ParseError(nm + " փոփոխականը դեռ սահմանված չէ։");

    auto varp = new Variable(nm);
    locals.push_back(varp);

    return varp;
}

//
Subroutine* Parser::getSubroutine(const std::string& nm, const std::vector<Expression*>& ags, bool func)
{
    // որոնել տրված անունով ենթածրագիրը արդեն սահմանվածների մեջ
    auto subrit = std::find_if(module->members.begin(), module->members.end(),
        [&nm](auto sp)->bool { return equalNames(sp->name, nm); });

    // եթե դեռ սահմանված չէ, պարզապես վերադաձնել @c nullptr
    if (subrit == module->members.end())
        return nullptr;

    // եթե օբյեկտը գտնվել է, ... 
    Subroutine* subr = *subrit;

    // ... ստուգել անունների տիպերի համընկնելը
    if (!equalTypes(subr->name, nm))
        throw TypeError("Ենթածրագրի անունը տարբերվում է կանչի անունից։");

    // ... ստուգել պարամետրերի և արգումենտների քանակների հավասար լինելը
    if (subr->parameters.size() != ags.size())
        throw ParseError("Ենթածրագրերի պարամետրերի և փոխանցված արգումենտների քանակները տարբեր են։");

    // ... ապա ստուգել պարամետրերի ու արգումենտների տիպրտի համապատասխանությունը
    for (int i = 0; i < subr->parameters.size(); ++i)
        if (typeOf(subr->parameters[i]) != ags[i]->type)
            throw TypeError(std::to_string(i)+ "-րդ պարամետրի ու արգումնտի տիպերը նույնը չեն։");

    // եթե հարցումը ֆունկցիայի կանչի համար է, բայց ենթածրագիրն արժեք չի վերադարձնում
    if (func && !subr->hasValue)
        throw ParseError(nm + " պրոցեդուրան արժեք չի վերադարձնում։");

    return subr;
}

//
void checkTypes(Binary* nodebi)
{
    Type tyo = nodebi->subexpro->type;
    Type tyi = nodebi->subexpri->type;
    Operation opc = nodebi->opcode;

    // տիպերի ստուգում և որոշում
    if (tyo == Type::Number && tyi == Type::Number) {
        if (opc == Operation::Conc)
            throw TypeError("'&' գործողությունը կիրառելի չէ թվերին։");
        else
            nodebi->type = Type::Number;
    }
    else if (tyo == Type::Text && tyi == Type::Text) {
        if (opc == Operation::Conc)
            nodebi->type = Type::Text;
        else if (opc >= Operation::Eq && opc <= Operation::Le)
            nodebi->type = Type::Number;
        else
            throw TypeError("'" + toString(opc) + "' գործողությունը կիրառելի չէ տեքստերին։");
    }
    else
        throw TypeError("'" + toString(opc) + "' գործողության երկու կողմերում տարբեր տիպեր են։");
}

//
bool equalNames(const std::string& no, const std::string& ni)
{
    std::string so = no, si = ni;
    if ('$' == so.back())
        so.pop_back();
    if ('$' == si.back())
        si.pop_back();
    return so == si;
}

//
bool equalTypes(const std::string& no, const std::string& ni)
{
    return typeOf(no) == typeOf(ni);
}
} // basic
