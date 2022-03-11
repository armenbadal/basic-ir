
#include "scanner.hxx"

namespace basic {

///
std::map<std::string_view, Token> Scanner::keywords{
    { "SUB",    Token::Subroutine },
    { "LET",    Token::Let },
    { "PRINT",  Token::Print },
    { "INPUT",  Token::Input },
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
    { "NOT",    Token::Not },
    { "TRUE",   Token::True },
    { "FALSE",  Token::False }
};

//
Scanner::Scanner(const std::filesystem::path& filename)
{
    // ստեղծել (բացել) ֆայլային հոսք
    source.open(filename);
    // կարդալիս բացատները չանտեսել
    source.unsetf(std::ios_base::skipws);
    // կարդալ առաջին նիշը
    source >> ch;
}

//
Scanner::~Scanner()
{
    // եթե ֆայլային հոսքը բացված է՝ փակել այն
    if( source.is_open() )
        source.close();
}

//! @brief Հերթական լեքսեմը կարդալու օպերատորը
Scanner& Scanner::operator>>(Lexeme& lex)
{
    next(lex);
    return *this;
}

//! @brief Հերթական լեքսեմը կարդալու ֆունկցիա
bool Scanner::next( Lexeme& lex )
{
    // լեքսեմի դաշտերի սկզբնական արժեքներ
    lex.kind = Token::None;
    lex.value = "";
    lex.line = line;

    // անտեսել բացատանիշերը (բացի նոր տողի նիշից)
    while( ch == ' ' || ch == '\t' || ch == '\r' )
        source >> ch;

    // ֆայլի վերջը
    if( source.eof() ) {
        lex.kind = Token::Eof;
        lex.value = "EOF";
        return true;
    }

    // երբ ընթացիկ նիշը թվանշան է՝ կարդալ թվային լիտերալ
    if( isdigit(ch) )
        return scanNumber(lex);

    // երբ ընթացիկ նիշը չակերտ է՝ կարդալ տողային լիտերալ
    if( ch == '"' )
        return scanText(lex);

    // երբ ընթացիկ նիշը տառ է՝ կարդալ իդենտիֆիկատոր կամ ծառայողական բառ
    if( isalpha(ch) )
        return scanIdentifier(lex);

    // երբ ընթացիկ նիշը ապաթարցն է, ...
    if( ch == '\'' ) {
        // ... ապա սկսվում է մեկնաբանություն, կարդալ ու 
        // դեն նետել մինչև տողի վերջը եղած բոլոր նիշերը
        while( ch != '\n' )
            source >> ch;
        // կարդալ ու վերադարձնել հաջորդ լեքսեմը
        return next(lex);
    }

    // երբ հանդիպել է նոր տողի նիշը
    if( ch == '\n' ) {
        lex.kind = Token::NewLine;
        lex.value = "\n";
        // փոխել ընթացիկ տողի համարը
        ++line;
        source >> ch;
        return true;
    }

    // «փոքր է», «փոքր է կամ հավասար» և «հավասար չէ» 
    // գործողությունները
    if( ch == '<' ) {
        lex.value = "<";
        source >> ch;
        if( ch == '>' ) {
            lex.value.push_back('>');
            source >> ch;
            lex.kind = Token::Ne;
        }
        else if( ch == '=' ) {
            lex.value.push_back('=');
            source >> ch;
            lex.kind = Token::Le;
        }
        else
            lex.kind = Token::Lt;
        return true;
    }

    // «մեծ է» և «մեծ է կամ հավասար» գործողությունները
    if( ch == '>' ) {
        lex.value = ">";
        source >> ch;
        if( ch == '=' ) {
            lex.value.push_back('=');
            source >> ch;
            lex.kind = Token::Ge;
        }
        else
            lex.kind = Token::Gt;
        return true;
    }

    // այլ մետասիմվոլներ
    switch( ch ) {
        case '(':
            lex.kind = Token::LeftPar;
            break;
        case ')':
            lex.kind = Token::RightPar;
            break;
        case ',':
            lex.kind = Token::Comma;
            break;
        case '+':
            lex.kind = Token::Add;
            break;
        case '-':
            lex.kind = Token::Sub;
            break;
        case '*':
            lex.kind = Token::Mul;
            break;
        case '/':
            lex.kind = Token::Div;
            break;
        case '^':
            lex.kind = Token::Pow;
            break;
        case '&':
            lex.kind = Token::Amp;
            break;
        case '=':
            lex.kind = Token::Eq;
            break;
    };
    // կարդալ հերթական նիշը
    source >> ch;

    // լեքսեմ կարդալու գործողությունը հաջողվել է, եթե kind-ը None չէ
    return lex.kind != Token::None;
}

//
bool Scanner::scanNumber(Lexeme& lex)
{
    // կարդալ թվանշանների շարք
    while( isdigit(ch) ) {
        lex.value.push_back(ch);
        source >> ch;
    }
    // եթե հերթական նիշը «.» է, ապա հանդիպել է
    // իրական թվի լիտերալ
    if( ch == '.' ) {
        // կետն ավելացնել լեքսեմի տեքստին
        lex.value.push_back('.');
        source >> ch;
        // նորից կարդալ թվանշանների հաջորդականություն
        while( isdigit(ch) ) {
            lex.value.push_back(ch);
            source >> ch;
        }
    }
    lex.kind = Token::Number;
    return true;
}

//
bool Scanner::scanText(Lexeme& lex)
{
    source >> ch;
    // քանի դեռ նորի չակերտ չի հանդիպել
    while( ch != '"' ) {
        // կարդալ ու հավաքել հերթական նիշերը
        lex.value.push_back(ch);
        source >> ch;
    }
    source >> ch;
    lex.kind = Token::Text;
    return true;
}

//
bool Scanner::scanIdentifier(Lexeme& lex)
{
    // կարդալ թվանշանների ու տառերի հաջորդականություն
    while( isalnum(ch) ) {
        lex.value.push_back(ch);
        source >> ch;
    }

    // եթե հանդիպել է «$» կամ «?», ապա դա էլ կցել լեքսեմի արժեքին
    if( ch == '$' || ch == '?' ) {
        lex.value.push_back(ch);
        source >> ch;
    }

    // լեքսեմի արժեքը փնտրել ծառայողական բառերի ցուցակում
    auto ival = keywords.find(lex.value);
    // եթե գտնվել է, ապա վերադարձնել համապատասխան պիտակը,
    // հակառակ դեպքում վերադարձնել իդենտիֆիկատորի պիտակ
    lex.kind = ival == keywords.end() ? Token::Identifier : ival->second;
    
    return true;
}

} // basic
