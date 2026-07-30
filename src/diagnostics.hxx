#pragma once

#include "lexeme.hxx"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace basic {

//! @brief Վերլուծության ժամանակ գրանցված սխալի նկարագրությունը
struct Diagnostic {
    unsigned int line = 0; //!< սխալի տողի համարը
    std::string message;   //!< սխալի հաղորդագրությունը

    std::string toString() const;
};

//! @brief Լեքսեմի ընթեռնելի ներկայացումը սխալի հաղորդագրության համար
//!
//! Անմիջապես @c value-ն օգտագործելը սխալ է. օրինակ՝ նոր տողի լեքսեմի արժեքը
//! հենց նոր տողի նիշն է, որը կոտրում է հաղորդագրությունը երկու մասի։
std::string describe(const Lexeme& lex);

//! @brief Սխալների հավաքիչը
//!
//! Կրկնվող հաղորդագրությունները զտվում են Վիրտի եղանակով (@c Mark
//! պրոցեդուրան, «Compiler construction», 7.3)․ սխալը գրանցվում է միայն
//! այն դեպքում, երբ վերլուծիչն առաջ է շարժվել վերջին գրանցված սխալից
//! հետո։ Դա բավական է մեկ սխալի հետևանքով առաջացող «սխալների հեղեղը»
//! կանխելու համար։
class Diagnostics {
public:
    //! @brief Հաղորդվող սխալների առավելագույն քանակը
    static constexpr std::size_t MaxErrors = 64;

    //! @brief Գրանցում է սխալը, եթե այն նոր տեղեկություն է պարունակում
    //!
    //! @param token սխալի տեղը ցույց տվող լեքսեմը
    //! @param message սխալի հաղորդագրությունը
    //! @param tokenIndex կլանված թոքենների քանակը՝ վերլուծիչի դիրքը
    void mark(const Lexeme& token, std::string_view message, std::size_t tokenIndex);

    bool hasErrors() const noexcept;
    const std::vector<Diagnostic>& all() const noexcept;

private:
    std::vector<Diagnostic> errors;
    std::size_t lastErrorToken = 0; //!< վերջին սխալի դիրքը
};

} // basic
