
#ifndef LEXEME_HXX
#define LEXEME_HXX

#include <string>
#include <vector>

namespace basic {
//! @brief Բառային տարրերի պիտակները
enum class Token : int {
    None, //!< ոչինչ

    Number,     //!< թվային հաստատուն
    Text,       //!< տեքստային հաստատուն
    Identifier, //!< իդենտիֆիկատոր

    Subroutine, //!< SUB
    Input,      //!< INPUT
    Print,      //!<
    Let,        //!< LET
    If,         //!< IF
    Then,       //!< THEN
    ElseIf,     //!< ELSEIF
    Else,       //!< ELSE
    While,      //!< WHILE
    For,        //!< FOR
    To,         //!< TO
    Step,       //!< STEP
    Call,       //!< CALL
    End,        //!< END

    NewLine, //!< նոր տողի նիշ

    Eq, //!< @c =
    Ne, //!< @c <>
    Lt, //!< @c <
    Le, //!< @c <=
    Gt, //!< @c >
    Ge, //!< @c >=

    LeftPar,  //!< @c (
    RightPar, //!< @c )
    Comma,    //!< @c ,

    Add, //!< @c +
    Sub, //!< @c -
    Amp, //!< @c &
    Or,  //!< OR
    Mul, //!< @c *
    Div, //!< @c /
    Mod, //!< MOD
    And, //!< AND
    Pow, //!< @c ^
    Not, //!< NOT

    Eof //!< ֆայլի վերջը
};

//! @brief Պիտակի տեքստային ներկայացումը
std::string toString(Token sym);

//! @brief Լեքսեմի դասը
//!
//! Օգտագործվում որպես բառային և շարահյուսական վերլուծիչների 
//! տվյալների փոխանակման միավոր։
//!
class Lexeme {
public:
    Token kind = Token::None; //!< պիտակը
    std::string value = ""; //!< տեքստը (լեքսեմը)
    unsigned int line = 0; //!< տողի համարը

public:
    Lexeme() = default;
    Lexeme(Token k, const std::string& v, unsigned int l);

    bool is(Token exp) const;
    bool is(const std::vector<Token>& exps) const;
    bool isIn(Token el, Token eh) const;

    //! @brief Լեքսեմի տեքստային ներկայացում
    //!
    //! Օգտագործվում է շտկման (debug) գործողությունների ժամանակ։
    std::string toString() const;
};
} // basic

#endif
