#include "diagnostics.hxx"

#include <format>

namespace basic {

std::ostream& operator<<(std::ostream& os, const Error& err)
{
    return os << std::get<0>(err) << ": " << std::get<1>(err);
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
    if( !_advanced )
        return;

    _advanced = false;
    ++_count;

    // ցուցակում պահում ենք միայն առաջին MaxErrors-ը, բայց հաշվում ենք բոլորը
    if( _count <= MaxErrors )
        _errors.push_back({line, std::string{message}});
}

const std::vector<Error>& Diagnostics::errors() const noexcept
{
    return _errors;
}

std::size_t Diagnostics::count() const noexcept
{
    return _count;
}

} // basic
