
#include "ast.hxx"
#include "lexeme.hxx"
#include "scanner.hxx"

namespace basic {
  //
  class Parser {
  private:
    Scanner& scanner;
    Lexeme lookahead;
    
  public:
    Parser( Scanner& scan );
    
  private:
    void match( Token tok );
    
    Program* parseProgram();
    Subroutine* parseSubroutine();

    Statement* parseStatements();
    Statement* parseInput();
    Statement* parsePrint();
    Statement* parseLet();
    Statement* parseIf();
    Statement* parseWhile();
    Statement* parseFor();
    Statement* parseCall();

    Expression* parseExpression();
    Expression* parseConjunction();
    Expression* parseEquality();
    Expression* parseComparison();
    Expression* parseAddition();
    Expression* parseMultiplication();
    Expression* parsePower();
    Expression* parseFactor();
    
    void parseNewLines();
  };
} // basic
