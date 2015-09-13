
#ifndef SCANNER_H
#define SCANNER_H

#include <fstream>
#include <functional>
#include <map>
#include <string>

#include "tokens.hxx"

namespace basic {
  class Lexeme {
  public:
    Token kind; // թոքենը
    std::string text; // բառը
    unsigned int line; // տողը

    /**/
    Lexeme() = default;

    /**/
    Lexeme( Token k, const std::string& v, unsigned int l )
      : kind{k}, text{v}, line{l}
    {}

    bool is( Token e )
    { return kind == e; }

    bool in( Token l, Token u )
    { return l <= kind && kind <= u; }
  };
}

namespace basic {
  /** @brief Նիշային վերլուծիչ
   *
   * Scanner դասը նախատեսված է BASIC-IR լեզվի նիշային
   * վերլուծության համար։ 
   */
  class Scanner {
  private:
    /// @brief Ֆայլի պարունակությունը
    char* source;
    /// @brief Հերթական նիշի դիրքը
    unsigned int position = 0;

    /// @brief Ընթացիկ տողի համարը
    unsigned int linenum = 1;
    /// @brief Կարդացած լեքսեմը
    std::string text = "";
    /// @brief Ծառայողական բառերի աղյուսակը
    static std::map<std::string,Token> keywords;

  public:
    /** @brief Կոնստրուկտոր
     *
     * @param name Ֆայլի անունը
     */
    Scanner( const std::string& name );
    ~Scanner();

    /** @brief Վերադարձնում է լեքսեմը */
    std::string lexeme() const { return text; }
    /** @brief Վերադարձնում է տողի համարը */
    const int line() const { return linenum; }
    /** @brief Վերադարձնում է հերթական թոքենը */
    Lexeme next();

  private:
    /** @brief հոսքից կարդում է տրված պրեդիկատին 
     * բավարարող նիշերի հաջորդականություն։ */
    std::string sequence( std::function<bool(char)> );
  };
}

#endif

