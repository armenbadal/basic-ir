
#ifndef LEXEME_HXX
#define LEXEME_HXX

#include <string>

namespace basic {
  //
  enum class Token {
    None
  };

  //
  class Lexeme {
  public:
    Token kind = Token::None;
    std::string value = "";
    unsigned int line = 0;

  public:
    Lexeme() = default;
    Lexeme( Token k, const std::string& v, unsigned int l );

    bool is( Token k ) const;
  };
} // basic
  
#endif

