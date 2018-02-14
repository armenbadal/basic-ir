
#include <exception>
#include <string>

#include "ast.hxx"
#include "lexeme.hxx"
#include "scanner.hxx"

namespace basic {
  //
  class Parser {
  private:
    Scanner scanner;
    Lexeme lookahead;

    Program* module = nullptr;
    Subroutine* cursubroutine = nullptr; // TODO: սա հեռացնել, պետք եղած դեպքում օգտագործել parseModule.back()-ը (?)
    
  public:
    Parser( const std::string& filename );
    Program* parse();
    
  private:
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
    Expression* parseAddition();
    Expression* parseMultiplication();
    Expression* parsePower();
    Expression* parseFactor();
    
    void parseNewLines();

    void match( Token tok );

    Type checkType( Operation op, Type left, Type right );
  };

  //
  class ParseError : std::exception {
  private:
    std::string message = "";
  public:
    ParseError( const std::string& mes )
      : message{mes}
    {}
    const char* what() const noexcept
    {
      return message.c_str();
    }
  };

  //
  class TypeError : std::exception {
  private:
    std::string message = "";
  public:
    TypeError( const std::string& mes )
      : message{mes}
    {}
    const char* what() const noexcept
    {
      return message.c_str();
    }
  };
} // basic

