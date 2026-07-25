#include "ast.hxx"
#include "lexeme.hxx"
#include "scanner.hxx"

#include <string>
#include <string_view>
#include <vector>

namespace basic {
class Parser {
public:
    Parser(Scanner& sc);
    ~Parser();

    Program::Ptr parse();

private:
    void parseProgram();
    void parseSubroutine();
    Statement::Ptr parseStatements();
    Statement::Ptr parseOneStatement();
    Statement::Ptr parseInput();
    Statement::Ptr parsePrint();
    Statement::Ptr parseLet();
    Statement::Ptr parseIf();
    Statement::Ptr parseElseChain();
    Statement::Ptr parseWhile();
    Statement::Ptr parseFor();
    Statement::Ptr parseCall();
    Expression::Ptr parseExpression();
    Expression::Ptr parseAddition();
    Expression::Ptr parseMultiplication();
    Expression::Ptr parsePower();
    Expression::Ptr parseFactor();
    void parseNewLines();
    std::string match(Token tok);

private:
    Scanner& scanner;
    Lexeme lookahead;
    std::vector<Subroutine::Ptr> subroutines;
};
} // basic
