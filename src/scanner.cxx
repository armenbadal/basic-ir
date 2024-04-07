
#include "scanner.hxx"

#include <cctype>
#include <string>

namespace basic {

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
    lex = next();
    return *this;
}

//! @brief Հերթական լեքսեմը կարդալու ֆունկցիա
Lexeme Scanner::next()
{
    // անտեսել բացատանիշերը (բացի նոր տողի նիշից)
    while( ch == ' ' || ch == '\t' || ch == '\r' )
        source >> ch;

    // ֆայլի վերջը
    if( source.eof() )
        return {Token::Eof, "EOF", line};

    // երբ ընթացիկ նիշը թվանշան է՝ կարդալ թվային լիտերալ
    if( isdigit(ch) )
        return scanNumber();

    // երբ ընթացիկ նիշը չակերտ է՝ կարդալ տողային լիտերալ
    if( ch == '"' )
        return scanText();

    // երբ ընթացիկ նիշը տառ է՝ կարդալ իդենտիֆիկատոր կամ ծառայողական բառ
    if( isalpha(ch) )
        return scanIdentifier();

    // երբ ընթացիկ նիշը ապաթարցն է, ...
    if( ch == '\'' ) {
        // ... ապա սկսվում է մեկնաբանություն, կարդալ ու 
        // դեն նետել մինչև տողի վերջը եղած բոլոր նիշերը
        while( ch != '\n' )
            source >> ch;
        // կարդալ ու վերադարձնել հաջորդ լեքսեմը
        return next();
    }

    // երբ հանդիպել է նոր տողի նիշը
    if( ch == '\n' ) {
        const auto pos = line;
        // փոխել ընթացիկ տողի համարը
        ++line;
        source >> ch;
        return {Token::NewLine, "\n", pos};
    }

    // «փոքր է», «փոքր է կամ հավասար» և «հավասար չէ» 
    // գործողությունները
    if( ch == '<' ) {
        Lexeme lex{Token::Lt, "<", line};

        source >> ch;
        if( ch == '>' ) {
            lex.kind = Token::Ne;
            lex.value = "<>";
            source >> ch;
        }
        else if( ch == '=' ) {
            lex.kind = Token::Le;
            lex.value = "<=";
            source >> ch;
        }
            
        return lex;
    }

    // «մեծ է» և «մեծ է կամ հավասար» գործողությունները
    if( ch == '>' ) {
        Lexeme lex{Token::Gt, ">", line};

        source >> ch;
        if( ch == '=' ) {
            lex.kind = Token::Ge;
            lex.value = ">=";
            source >> ch;
        }

        return lex;
    }

    // այլ մետասիմվոլներ
    const auto msi = metasymbols.find(ch);
    auto kind = msi == metasymbols.end() ? Token::None : msi->second;
    Lexeme lex{kind, std::string{ch}, line};

    // կարդալ հերթական նիշը
    source >> ch;

    return lex;
}

//
Lexeme Scanner::scanNumber()
{
    const auto pos = line;
    std::string value;

    // կարդալ թվանշանների շարք
    while( isdigit(ch) ) {
        value += ch;
        source >> ch;
    }
    // եթե հերթական նիշը «.» է, ապա հանդիպել է
    // իրական թվի լիտերալ
    if( ch == '.' ) {
        // կետն ավելացնել լեքսեմի տեքստին
        value += '.';
        source >> ch;
        // նորից կարդալ թվանշանների հաջորդականություն
        while( isdigit(ch) ) {
            value += ch;
            source >> ch;
        }
    }
    
    return {Token::Number, value, pos};
}

//
Lexeme Scanner::scanText()
{
    const auto pos = line;
    std::string value;

    source >> ch;
    // քանի դեռ նորից չակերտ չի հանդիպել
    while( ch != '"' ) {
        // կարդալ ու հավաքել հերթական նիշերը
        value += ch;
        source >> ch;
    }
    source >> ch;

    return {Token::Text, value, pos};
}

//
Lexeme Scanner::scanIdentifier()
{
    const auto pos = line;
    std::string value;

    // կարդալ թվանշանների ու տառերի հաջորդականություն
    while( isalnum(ch) ) {
        value += ch;
        source >> ch;
    }

    // եթե հանդիպել է «$» կամ «?», ապա դա էլ կցել լեքսեմի արժեքին
    if( ch == '$' || ch == '?' ) {
        value += ch;
        source >> ch;
    }

    // լեքսեմի արժեքը փնտրել ծառայողական բառերի ցուցակում
    auto ival = keywords.find(value);
    // եթե գտնվել է, ապա վերադարձնել համապատասխան պիտակը,
    // հակառակ դեպքում վերադարձնել իդենտիֆիկատորի պիտակ
    auto kind = ival == keywords.end() ? Token::Identifier : ival->second;
    
    return {kind, value, pos};
}

} // namespace basic
