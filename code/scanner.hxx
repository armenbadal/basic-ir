
#ifndef SCANNER_H
#define SCANNER_H

#include <fstream>
#include <functional>
#include <map>
#include <string>

#include "tokens.hxx"

/**/
class Scanner {
private:
  std::ifstream source;
  char c;

  int linenum{1};
  std::string text{""};
  static std::map<std::string,Token> keywords;

public:
  Scanner(const std::string&);
  std::string lexeme() const;
  inline int line() const { return linenum; }
  Token next();

private:
  std::string sequence(std::function<bool(char)>);
};

#endif

