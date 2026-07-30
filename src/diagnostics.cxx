#include "diagnostics.hxx"

#include <format>

namespace basic {

std::string Diagnostic::toString() const
{
    return std::format("{}: {}", line, message);
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

void Diagnostics::mark(const Lexeme& token, std::string_view message, std::size_t tokenIndex)
{
    // Վիրտի կանոնը. եթե վերլուծիչը տեղից չի շարժվել, ուրեմն այս սխալը
    // նախորդի կրկնությունն է
    if( tokenIndex <= lastErrorToken )
        return;

    lastErrorToken = tokenIndex;

    if( errors.size() >= MaxErrors ) {
        if( errors.size() == MaxErrors )
            errors.push_back({token.line, "Չափազանց շատ սխալներ։"});
        return;
    }

    errors.push_back({token.line, std::string{message}});
}

bool Diagnostics::hasErrors() const noexcept
{
    return !errors.empty();
}

const std::vector<Diagnostic>& Diagnostics::all() const noexcept
{
    return errors;
}

} // basic
