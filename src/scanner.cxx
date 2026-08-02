
#include "scanner.hxx"

#include <cctype>
#include <string>
#include <concepts>

namespace basic {

// ծառայողական բառերի ցուցակ
static inline std::map<std::string_view, Token> keywords{
    { "SUB",    Token::Subroutine },
    { "LET",    Token::Let },
    { "DIM",    Token::Dim },
    { "AS",     Token::As },
    { "IF",     Token::If },
    { "THEN",   Token::Then },
    { "ELSEIF", Token::ElseIf },
    { "ELSE",   Token::Else },
    { "WHILE",  Token::While },
    { "FOR",    Token::For },
    { "TO",     Token::To },
    { "STEP",   Token::Step },
    { "CALL",   Token::Call },
    { "END",    Token::End },
    { "MOD",    Token::Mod },
    { "AND",    Token::And },
    { "OR",     Token::Or },
    { "NOT",    Token::Not }
};

// մետասիմվոլների ցուցակ
static inline std::map<char, Token> metasymbols{
    { '(', Token::LeftPar },
    { ')', Token::RightPar },
    { '[', Token::LeftBrack },
    { ']', Token::RightBrack },
    { ',', Token::Comma },
    { '+', Token::Add },
    { '-', Token::Sub },
    { '*', Token::Mul },
    { '/', Token::Div },
    { '\\', Token::Quot },
    { '^', Token::Pow },
    { '&', Token::Amp },
    { '=', Token::Eq }
};

template<typename Predicate>
requires std::predicate<Predicate, char>
std::string readWhile(std::istream& input, Predicate predicate)
{
    std::string result;
    char ch = '\0';
    while( input >> ch ) {
        if( !predicate(ch) ) {
            input.putback(ch);
            break;
        }
        result += ch;
    }
    return result;
}

bool isAlpha(char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

bool isDigit(char c)
{
    return c >= '0' && c <= '9';
}

bool isAlnum(char c)
{
    return isAlpha(c) || isDigit(c);
}

bool isSpace(char c)
{
    return c == ' ' || c == '\t' || c == '\r';
}


Scanner::Scanner(std::istream& input)
    : source{input}
{
    // կարդալիս բացատները չանտեսել
    source.unsetf(std::ios_base::skipws);
}

// Հերթական լեքսեմը կարդալու ֆունկցիա
Lexeme Scanner::scan()
{
    readWhile(source, isSpace);

    // մեկնաբանություններ
    if( source.peek() == '\'' ) {
        readWhile(source, [](char c) { return c != '\n'; });
        return scan();
    }

    // ֆայլի վերջը
    if( source.peek() == std::char_traits<char>::eof() )
        return { Token::Eof, "EOF", line };

    // երբ ընթացիկ նիշը թվանշան է՝ կարդալ թվային լիտերալ
    if( isDigit(source.peek()) )
        return scanNumber();

    // երբ ընթացիկ նիշը չակերտ է՝ կարդալ տողային լիտերալ
    if( source.peek() == '"')
        return scanText();

    // երբ ընթացիկ նիշը տառ է՝ կարդալ իդենտիֆիկատոր կամ ծառայողական բառ
    if( std::isalpha(source.peek()) )
        return scanIdentifier();

    // երբ հանդիպել է նոր տողի նիշը
    if( char ch = source.peek(); ch == '\n' ) {
        source.ignore();
        const auto pos = line;
        // փոխել ընթացիկ տողի համարը
        ++line;
        return { Token::NewLine, "\n", pos };
    }

    // «փոքր է», «փոքր է կամ հավասար» և «հավասար չէ» 
    // գործողությունները
    if( source.peek() == '<') {
        source.ignore();
        Lexeme lex{ Token::Lt, "<", line };

        if( !source.eof() && source.peek() == '>') {
            source.ignore();
            lex.kind = Token::Ne;
            lex.value = "<>";
        }
        else if (!source.eof() && source.peek() == '=') {
            source.ignore();
            lex.kind = Token::Le;
            lex.value = "<=";
        }

        return lex;
    }

    // «մեծ է» և «մեծ է կամ հավասար» գործողությունները
    if( source.peek() == '>') {
        source.ignore();
        Lexeme lex{ Token::Gt, ">", line };

        if( !source.eof() && source.peek() == '=' ) {
            source.ignore();
            lex.kind = Token::Ge;
            lex.value = ">=";
        }

        return lex;
    }

    // այլ մետասիմվոլներ
    const auto msi = metasymbols.find(source.peek());
    auto kind = msi == metasymbols.end() ? Token::None : msi->second;
    char ch = source.get();
    Lexeme lex{ kind, std::string{ch}, line};

    return lex;
}


Lexeme Scanner::scanNumber()
{
    const auto pos = line;
    // կարդալ թվանշանների շարք
    std::string value = readWhile(source, isDigit);
    
    // եթե հերթական նիշը «.» է, ապա հանդիպել է իրական թվի լիտերալ
    if (!source.eof() && source.peek() == '.') {
        source.get();
        value += '.';
        value += readWhile(source, isDigit);
    }

    return { Token::RealLit, value, pos };
}


Lexeme Scanner::scanText()
{
    source.ignore();
    const auto pos = line;
    std::string value = readWhile(source, [](char c) { return c != '"'; });
    if( !source.eof() && source.peek() == '"' )
        source.ignore();
    return { Token::TextLit, value, pos };
}


Lexeme Scanner::scanIdentifier()
{
    const auto pos = line;
    // կարդալ թվանշանների ու տառերի հաջորդականություն
    std::string value = readWhile(source, isAlnum);

    if( value == "TRUE" || value == "FALSE" )
        return { Token::BoolLit, value, pos };

    // լեքսեմի արժեքը փնտրել ծառայողական բառերի ցուցակում
    auto ival = keywords.find(value);
    // եթե գտնվել է, ապա վերադարձնել համապատասխան պիտակը,
    // հակառակ դեպքում վերադարձնել իդենտիֆիկատորի պիտակ
    auto kind = ival == keywords.end() ? Token::Identifier : ival->second;

    return { kind, value, pos };
}

} // namespace basic
