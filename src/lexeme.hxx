
#ifndef LEXEME_HXX
#define LEXEME_HXX

#include <string>

namespace basic {
  ///
  enum class Token : int {
    None,

    Real,
    Text,
    Identifier,

    Declare,
    Function,
    Input,
    Print,
    Let,
    If,
    Then,
    ElseIf,
    Else,
    While,
    For,
    To,
    Step,
    Call,
    End,

    NewLine,

    Eq,
    Ne,
    Lt,
    Le,
    Gt,
    Ge,

    LeftPar,
    RightPar,
    Comma,

    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Pow,

    And,
    Or,
    Not,

    Eof
  };

  ///
  class Lexeme {
  public:
    Token kind = Token::None;
    std::string value = "";
    unsigned int line = 0;
  
  public:
    Lexeme() = default;
    Lexeme( Token t, const std::string& v, unsigned int l );

    bool is(Token exp);

    std::string toString() const;
  };
} // basic
  
#endif

