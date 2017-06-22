
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

  public:
    Program* parseProgram();

  private:
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
    Expression* parseAddition();
    Expression* parseMultiplication();
    Expression* parsePower();
    Expression* parseFactor();
    
    void parseNewLines();

    void match( Token tok );
  };
} // basic
