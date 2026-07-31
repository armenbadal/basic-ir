#include "diagnostics.hxx"

#include <format>

namespace basic {

std::ostream& operator<<(std::ostream& os, const SyntaxError& err)
{
    return os << err.line << ": " << err.message;
}

std::string describe(const Lexeme& lex)
{
    switch( lex.kind ) {
        case Token::NewLine:
            return "տողի ավարտ";
        case Token::Eof:
            return "ֆայլի ավարտ";
        case Token::None:
            return std::format("անհայտ նիշ '{}'", lex.value);
        case Token::Number:
        case Token::Text:
        case Token::Identifier:
            return std::format("'{}'", lex.value);
        default:
            return std::format("'{}'", toString(lex.kind));
    }
}

void Diagnostics::mark(unsigned int line, std::string_view message)
{
    if( !_advanced || _errors.size() >= MaxErrors )
        return;

    _advanced = false;
    _errors.push_back({line, std::string{message}});
}

const std::vector<SyntaxError>& Diagnostics::errors() const noexcept
{
    return _errors;
}

} // basic
