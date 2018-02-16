
#include "lexeme.hxx"

namespace basic {
//
Lexeme::Lexeme(Token k, const std::string& v, unsigned int l)
    : kind{ k }, value{ v }, line{ l }
{
}

//
bool Lexeme::is(Token exp) const
{
    return exp == kind;
}

//
bool Lexeme::is(const std::vector<Token>& exps) const
{
    for (auto k : exps)
        if (k == kind)
            return true;
    return false;
}

//
bool Lexeme::isIn(Token el, Token eh) const
{
    return kind >= el && kind <= eh;
}

//
std::string Lexeme::toString() const
{
    return "<" + value + ", " + std::to_string(line) + ">";
}
} // basic
