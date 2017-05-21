#include "Lexeme.h"
#include "Scanner.h"

namespace basic {
class Parser {
private:
    Scanner& scanner;
    Lexeme lookahead;

public:
    Parser(Scanner& scan);

private:
    void match(Token tok);

    void parseProgram();
    void parseDeclare();
    void parseFunction();
    void parseHeader();
    void parseCommand();
    void parseInput();
    void parsePrint();
    void parseLet();
    void parseIf();
    void parseWhile();
    void parseFor();
    void parseCall();

    void parseNewLines();
};

}
