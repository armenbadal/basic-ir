
#include "lexeme.hxx"

namespace basic {
  //
  Lexeme::Lexeme( Token k, const std::string& v, unsigned int l )
    : kind{k}, value{v}, line{l}
  {}

  //
  bool Lexeme::is( Token k ) const
  {
    return k == kind;
  }
  
  //
  std::string Lexeme::toString() const
  {
    return "<" + value + ", " + std::to_string(line) + ">";
  }
} // basic

