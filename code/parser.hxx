#ifndef PARSER_H
#define PARSER_H

#include <set>
#include <string>
#include <utility>
#include <vector>

#include "tokens.hxx"
#include "scanner.hxx"
#include "ast.hxx"

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
  std::pair<std::string,std::string> parseNameDecl();
  void parseType();
  void parseDeclare();
  Function* parseSubrHeader();
  Function* parseSubroutine();
  Function* parseFuncHeader();
  Function* parseFunction();
  Statement* parseStatement();
  Statement* parseSequence();
  Statement* parseDim();
  Statement* parseLet();
  Statement* parseIf();
  Statement* parseFor();
  Statement* parseWhile();
  Expression* parseRelation();
  Expression* parseExpression();
  Expression* parseTerm();
  Expression* parsePower();
  Expression* parseFactor();
};

#endif

