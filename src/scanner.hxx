
#ifndef SCANNER_HXX
#define SCANNER_HXX

#include <fstream>
#include <functional>
#include <map>
#include <string>

#include "lexeme.hxx"

namespace basic {
//
class Scanner {
private:
    static std::map<const std::string, Token> keywords;

private:
    std::ifstream source;
    char ch;

    unsigned int line = 1;

public:
    Scanner(const std::string& filename);
    ~Scanner();

    Scanner& operator>>(Lexeme& lex);

private:
    void sequence(std::function<bool(char)> pred, std::string& res);
    bool next(Lexeme& lex);
    bool scanNumber(Lexeme& lex);
    bool scanText(Lexeme& lex);
    bool scanIdentifier(Lexeme& lex);
};
} // basic

#endif
