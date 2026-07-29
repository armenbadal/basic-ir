#pragma once

#include <string>

namespace basic {

//! @brief Բառային տարրերի պիտակները
enum class Token : int {
    None, //!< ոչինչ

    Number,     //!< թվային հաստատուն
    Text,       //!< տեքստային հաստատուն
    Identifier, //!< իդենտիֆիկատոր
    True,       //!< @c TRUE հաստատունը
    False,      //!< @c FALSE հաստատունը

    Subroutine, //!< @c SUB
    Input,      //!< @c INPUT
    Print,      //!< @c PRINT
    Dim,        //!< @c DIM
    Let,        //!< @c LET
    If,         //!< @c IF
    Then,       //!< @c THEN
    ElseIf,     //!< @c ELSEIF
    Else,       //!< @c ELSE
    While,      //!< @c WHILE
    For,        //!< @c FOR
    To,         //!< @c TO
    Step,       //!< @c STEP
    Call,       //!< @c CALL
    End,        //!< @c END

    NewLine, //!< նոր տողի նիշ

    Eq, //!< @c =
    Ne, //!< @c <>
    Lt, //!< @c <
    Le, //!< @c <=
    Gt, //!< @c >
    Ge, //!< @c >=

    LeftPar,    //!< @c (
    RightPar,   //!< @c )
    LeftBrack,  //< @c [
    RightBrack, //< @c ]
    Comma,      //!< @c ,

    Add, //!< @c +
    Sub, //!< @c -
    Amp, //!< @c &
    Or,  //!< @c OR
    Mul, //!< @c *
    Div, //!< @c /
    Mod, //!< @c MOD
    Quot, //!< @c '\'
    And, //!< @c AND
    Pow, //!< @c ^
    Not, //!< @c NOT

    Eof //!< ֆայլի վերջը
};

//! @brief Պիտակի տեքստային ներկայացումը
std::string toString(Token sym);

//! @brief Լեքսեմի դասը
//!
//! Օգտագործվում է որպես բառային և շարահյուսական վերլուծիչների 
//! տվյալների փոխանակման միավոր։
//!
class Lexeme {
public:
    Token kind = Token::None; //!< պիտակը
    std::string value;   //!< տեքստը (լեքսեմը)
    unsigned int line = 0;    //!< տողի համարը

public:
    Lexeme() = default;
    Lexeme(Token k, std::string v, unsigned int l)
        : kind{ k }, value{ std::move(v) }, line{ l }
    {}

    bool is(Token exp) const
    {
        return exp == kind;
    }

    template<typename... Tokens>
    bool is(Token ex, Tokens... exps) const
    {
        return is(ex) || is(exps...);
    }

    bool isIn(Token el, Token eh) const
    {
        return kind >= el && kind <= eh;
    }

    //! @brief Լեքսեմի տեքստային ներկայացում
    //!
    //! Օգտագործվում է շտկման (debug) գործողությունների ժամանակ։
    std::string toString() const;
};
} // basic
