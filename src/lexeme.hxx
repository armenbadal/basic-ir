
#ifndef LEXEME_HXX
#define LEXEME_HXX

#include <string>
#include <vector>

namespace basic {
  ///
  enum class Token : int {
    None,

    Number,
    Text,
    Identifier,

    Subroutine,
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
    Amp,
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
    Lexeme( Token k, const std::string& v, unsigned int l );

    bool is( Token exp ) const;
    bool is( const std::vector<Token>& exps ) const;

    std::string toString() const;
  };
} // basic
  
#endif

