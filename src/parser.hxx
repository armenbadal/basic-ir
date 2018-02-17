
#include <exception>
#include <string>

#include "ast.hxx"
#include "lexeme.hxx"
#include "scanner.hxx"

namespace basic {
//
class Parser {
private:
    Program* module = nullptr;

    Scanner scanner;
    Lexeme lookahead;

    // անորոշ հղումներ. բանալին ենթածրագրի անունն է,
    // իսկ արժեքը դրան հղվող Apply օբյեկտների ցուցակը
    std::map<std::string, std::list<Apply*>> unresolved;

public:
    Parser(const std::string& filename);
    ~Parser();

    Program* parse();

private:
    void parseProgram();
    void parseSubroutine();

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

    void match(Token tok);

    Type checkType(Operation op, Type left, Type right);

    Variable* getVariable(const std::string& nm);
};

//
void checkTypes(Binary* nodebi);
bool equalNames(const std::string& no, const std::string& ni);
bool equalTypes(const std::string& no, const std::string& ni);
} // basic
