#ifndef PARSER_H
#define PARSER_H

#include <set>
#include <string>
#include <tuple>
#include <vector>
#include <utility>

#include "tokens.hxx"
#include "scanner.hxx"


/**/
class Parser {
private:
  Scanner sc;
  Token lookahead;

public:
  Parser( const char* );
  void parse(); // TODO create AST

private:
  static std::set<Token> FD;
  static std::set<Token> FS;
  bool inSet( const std::set<Token>& );

private:
  void match( Token );
  void parseEol();
  void parseGlobal();
  void parseStatement();
  void parseSequence();
  void parseDim();
  void parseNameDecl();
  void parseType();
  void parseDeclare();
  std::tuple<std::string,std::pair<std::string,std::string>>> parseSubrHeader();
  void parseSubroutine();
  void parseFuncHeader();
  void parseFunction();
  void parseLet();
  void parseIf();
  void parseFor();
  void parseWhile();
};

#endif

