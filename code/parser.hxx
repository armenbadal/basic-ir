#ifndef PARSER_H
#define PARSER_H

#include <set>
#include <string>
#include <utility>
#include <vector>

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
  void parseEols();
  void parseGlobal();
  void parseStatement();
  void parseSequence();
  void parseDim();
  std::pair<std::string,std::string> parseNameDecl();
  void parseType();
  void parseDeclare();
  void parseSubrHeader();
  void parseSubroutine();
  void parseFuncHeader();
  void parseFunction();
  void parseLet();
  void parseIf();
  void parseFor();
  void parseWhile();
  void parseRelation();
  void parseExpression();
  void parseTerm();
  void parseFactor();
};

#endif

