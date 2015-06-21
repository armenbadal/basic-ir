
#ifndef SCANNER_H
#define SCANNER_H

#include <fstream>
#include <functional>
#include <map>
#include <string>

#include "tokens.hxx"

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
  size_t position = 0;

  /// @brief Ընթացիկ տողի համարը
  int linenum = 1;
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
  Token next();

private:
  /** @brief հոսքից կարդում է տրված պրեդիկատին 
   * բավարարող նիշերի հաջորդականություն։ */
  std::string sequence( std::function<bool(char)> );
};

#endif

