
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
    Subroutine* parseDeclare();
    Subroutine* parseSubroutine();
    Subroutine* parseHeader();
    Statement* parseStatement();
    Input* parseInput();
    Print* parsePrint();
    Let* parseLet();
    If* parseIf();
    While* parseWhile();
    For* parseFor();
    Call* parseCall();

    void parseNewLines();
  };
} // basic
