#pragma once

#include "lexeme.hxx"

#include <cstddef>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

namespace basic {

// Վերլուծության սխալ
struct SyntaxError {
    unsigned int line = 0; // տողի համարը
    std::string message;   // հաղորդագրությունը
};

std::ostream& operator<<(std::ostream& os, const SyntaxError& err);

// Լեքսեմի ընթեռնելի ներկայացումը սխալի հաղորդագրության համար
std::string describe(const Lexeme& lex);

class Diagnostics {
public:
    // Հաղորդվող սխալների առավելագույն քանակը
    static constexpr std::size_t MaxErrors = 8;

    // Գրանցում է սխալը, եթե այն նոր տեղեկություն է պարունակում
    void mark(unsigned int line, std::string_view message);

    // Նշում է, որ թոքեն է սպառվել (Վիրտի կանոնի համար)
    void advance() noexcept { _advanced = true; }

    const std::vector<SyntaxError>& errors() const noexcept;

private:
    std::vector<SyntaxError> _errors;
    bool _advanced = true;
};

} // basic
