#pragma once

#include "lexeme.hxx"

#include <cstddef>
#include <ostream>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace basic {

// Վերլուծության սխալ
using Error = std::tuple<unsigned int, std::string>;

std::ostream& operator<<(std::ostream& os, const Error& err);

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

    // Առաջին MaxErrors սխալները
    const std::vector<Error>& errors() const noexcept;

    // Գտնված սխալների ընդհանուր քանակը. կարող է errors()-ից մեծ լինել
    std::size_t count() const noexcept;

private:
    std::vector<Error> _errors;
    std::size_t _count = 0;
    bool _advanced = true;
};

} // basic
