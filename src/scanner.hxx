
#ifndef SCANNER_HXX
#define SCANNER_HXX

#include <filesystem>
#include <fstream>
#include <map>
#include <string_view>

#include "lexeme.hxx"

namespace basic {
//! @brief Բառային վերլուծիչ
class Scanner {
public:
    //! @brief Բառային վերլուծիչի կոնստրուկտոր
    //!
    //! @param filename Վերլուծվելիք ֆայլի ճանապարհը
    explicit Scanner(const std::filesystem::path& filename);

    //! @brief Բառային վերլուծիչի դեստրուկտոր
    ~Scanner();

    //! @brief Հերթական լեքսեմը կարդալու օպերատոր
    Scanner& operator>>(Lexeme& lex);

private:
    //! @brief Հերթական լեքսեմը կարդալու ֆունկցիա
    Lexeme next();

    //! @brief Թվային լիտերալ
    Lexeme scanNumber();

    //! @brief Տեքստային լիտերալ
    Lexeme scanText();

    //! @brief Իդենտիֆիկատոր
    Lexeme scanIdentifier();

private:
    std::ifstream source; //!< նիշերը կարդալու հոսքը
    char ch = '\0'; //!< ընթացիկ նիշը

    unsigned int line = 1; //!< ընթացիկ տողը

private:
    //! @brief ծառայողական բառերի ցուցակ
    static inline std::map<std::string_view,Token> keywords{
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

    //! @brief մետասիմվոլների ցուցակ
    static inline std::map<char,Token> metasymbols{
        { '(', Token::LeftPar },
        { ')', Token::RightPar },
        { ',', Token::Comma },
        { '+', Token::Add },
        { '-', Token::Sub },
        { '*', Token::Mul },
        { '/', Token::Div },
        { '^', Token::Pow },
        { '&', Token::Amp },
        { '=', Token::Eq }
    };
};

} // basic

#endif
