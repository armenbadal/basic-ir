#ifndef PARSER_H
#define PARSER_H

#include <set>
#include <string>
#include <utility>
#include <vector>

#include "ast.hxx"
#include "scanner.hxx"
#include "symtab.hxx"
#include "tokens.hxx"


/**/
class Parser {
private:
  std::string file;
  Scanner sc;
  Token lookahead;
  SymbolTable*  symtab;

public:
  Parser(const char* nm) 
    : file{std::string{nm}}, sc{file}
  {
    symtab = new SymbolTable();
  }
  ~Parser()
  {
    delete symtab;
  }
  Module* parse();

private:
  static std::set<Token> FD;
  static std::set<Token> FS;
  static std::set<Token> FF;
  bool inSet( const std::set<Token>& );

private:
  void match( Token );
  void parseEols();
  Symbol parseNameDecl();
  std::string parseDeclList(std::vector<Symbol>&);
  void parseType();
  Function* parseDeclare();
  Function* parseSubrHeader();
  Function* parseSubroutine();
  Function* parseFuncHeader();
  Function* parseFunction();
  Statement* parseStatement();
  Statement* parseSequence();
  Statement* parseDim();
  Statement* parseAssignment();
  Statement* parseLet();
  Statement* parseSubCall();
  Statement* parseIf();
  Statement* parseFor();
  Statement* parseWhile();
  Statement* parseInput();
  Statement* parsePrint();
  Statement* parseReturn();
  Expression* parseDisjunction();
  Expression* parseConjunction();
  Expression* parseEquality();
  Expression* parseRelation();
  Expression* parseAddition();
  Expression* parseMultiplication();
  Expression* parsePower();
  Expression* parseFactor();
  Expression* parseVariableOrFuncCall();
};

#endif

