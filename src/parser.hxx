
#include <exception>
#include <string>

#include "ast.hxx"
#include "lexeme.hxx"
#include "scanner.hxx"

namespace basic {
//! @brief Շարահյուսական վերլուծիչը
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

    //! @brief Հայտարարում է BASIC-IR լեզվի ներդրված ենթածրագիր
    //!
    //! @param nm - ենթածրագրի անունը
    //! @param ps - պարամետերեի ցուցակը
    //! @param rv - ֆունկցիա է կամ պրոցեդուրա
    //!
    void declareBuiltIn(const std::string& nm, const std::vector<std::string>& ps, bool rv);
    Variable* getVariable(const std::string& nm, bool rval);
    Subroutine* getSubroutine(const std::string& nm, const std::vector<Expression*>& ags, bool func);
};

// TODO: վերանայել այս դասերը
//
class ParseError : public std::exception {
private:
    std::string message = "";

public:
    ParseError(const std::string& mes)
        : message(mes)
    {}

    ParseError(Token exp, Token got)
    {
        message = "սպասվում էր ..., բայց հանդիպել է ...";
    }

    const char* what() const noexcept
    {
        return message.c_str();
    }
};

//
class TypeError : public std::exception {
private:
    std::string message = "";

public:
    TypeError(const std::string& mes)
        : message(mes)
    {
    }
    const char* what() const noexcept
    {
        return message.c_str();
    }
};

//
void checkTypes(Binary* nodebi);
bool equalNames(const std::string& no, const std::string& ni);
bool equalTypes(const std::string& no, const std::string& ni);
} // basic
