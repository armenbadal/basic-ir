#pragma once

#include <string>

namespace basic {

// Բառային տարրերի պիտակները
enum class Token : int {
    None, // ոչինչ

    Identifier, // իդենտիֆիկատոր
    RealLit,    // թվային հաստատուն
    TextLit,    // տեքստային հաստատուն
    BoolLit,    // բուլյան հաստատուն

    Subroutine, // SUB
    Dim,        // DIM
    As,         // AS
    Let,        // LET
    If,         // IF
    Then,       // THEN
    ElseIf,     // ELSEIF
    Else,       // ELSE
    While,      // WHILE
    For,        // FOR
    To,         // TO
    Step,       // STEP
    Call,       // CALL
    End,        // END

    Real,       // REAL
    Text,       // TEXT
    Bool,       // BOOL

    NewLine, // նոր տողի նիշ

    Eq, // =
    Ne, // <>
    Lt, // <
    Le, // <=
    Gt, // >
    Ge, // >=

    LeftPar,    // (
    RightPar,   // )
    LeftBrack,  //< [
    RightBrack, //< ]
    Comma,      // ,

    Add, // +
    Sub, // -
    Amp, // &
    Or,  // OR
    Mul, // *
    Div, // /
    Mod, // MOD
    Quot, // '\'
    And, // AND
    Pow, // ^
    Not, // NOT

    Eof // ֆայլի վերջը
};

// Պիտակի տեքստային ներկայացումը
std::string toString(Token sym);

// Լեքսեմի դասը
//
// Օգտագործվում է որպես բառային և շարահյուսական վերլուծիչների 
// տվյալների փոխանակման միավոր։
//
class Lexeme {
public:
    Token kind = Token::None; // պիտակը
    std::string value;   // տեքստը (լեքսեմը)
    unsigned int line = 0;    // տողի համարը

public:
    Lexeme() = default;
    Lexeme(Token k, std::string v, unsigned int l)
        : kind{k}, value{std::move(v)}, line{l}
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

    // Լեքսեմի տեքստային ներկայացում
    //
    // Օգտագործվում է շտկման (debug) գործողությունների ժամանակ։
    std::string toString() const;
};
} // basic
