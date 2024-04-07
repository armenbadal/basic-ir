
#include "lexeme.hxx"

#include <format>
#include <map>
#include <memory>
#include <ranges>

namespace basic {
//
std::string toString(Token sym)
{
    static std::map<Token, std::string> names{
        { Token::None, "None" },
        { Token::Number, "Number" },
        { Token::Text, "Text" },
        { Token::True, "TRUE" },
        { Token::False, "FALSE" },
        { Token::Identifier, "IDENT" },
        { Token::Subroutine, "SUB" },
        { Token::Input, "INPUT" },
        { Token::Print, "PRINT" },
        { Token::Let, "LET" },
        { Token::If, "IF" },
        { Token::Then, "THEN" },
        { Token::ElseIf, "ELSEIF" },
        { Token::Else, "ELSE" },
        { Token::While, "WHILE" },
        { Token::For, "FOR" },
        { Token::To, "TO" },
        { Token::Step, "STEP" },
        { Token::Call, "CALL" },
        { Token::End, "END" },
        { Token::NewLine, "New Line" },
        { Token::Eq, "=" },
        { Token::Ne, "<>" },
        { Token::Lt, "<" },
        { Token::Le, "<=" },
        { Token::Gt, ">" },
        { Token::Ge, ">=" },
        { Token::LeftPar, "(" },
        { Token::RightPar, ")" },
        { Token::Comma, "," },
        { Token::Add, "+" },
        { Token::Sub, "-" },
        { Token::Amp, "&" },
        { Token::Or, "OR" },
        { Token::Mul, "*" },
        { Token::Div, "/" },
        { Token::Mod, "MOD" },
        { Token::And, "AND" },
        { Token::Pow, "^" },
        { Token::Not, "NOT" },
        { Token::Eof, "Eof" }
    };

    return names[sym];
}

//
Lexeme::Lexeme(Token k, std::string v, unsigned int l)
    : kind{k}, value{std::move(v)}, line{l}
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
    return exps.end() != std::ranges::find(exps, kind);
}

//
bool Lexeme::isIn(Token el, Token eh) const
{
    return kind >= el && kind <= eh;
}

//
std::string Lexeme::toString() const
{
    return std::format("<{}, {}>", value, line);
}

} // basic
