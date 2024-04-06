
#ifndef SCANNER_HXX
#define SCANNER_HXX

#include <filesystem>
#include <fstream>
#include <map>
#include <string_view>

#include "lexeme.hxx"

namespace basic {
//! @brief Բառային վերլուծիչ
class Scanner {
public:
    //! @brief Բառային վերլուծիչի կոնստրուկտոր
    //!
    //! @param filename Վերլուծվելիք ֆայլի ճանապարհը
    explicit Scanner(const std::filesystem::path& filename);

    //! @brief Բառային վերլուծիչի դեստրուկտոր
    ~Scanner();

    //! @brief Հերթական լեքսեմը կարդալու օպերատոր
    Scanner& operator>>(Lexeme& lex);

private:
    //! @brief Հերթական լեքսեմը կարդալու ֆունկցիա
    Lexeme next();

    //! @brief Թվային լիտերալ
    Lexeme scanNumber();

    //! @brief Տեքստային լիտերալ
    Lexeme scanText();

    //! @brief Իդենտիֆիկատոր
    Lexeme scanIdentifier();

private:
    //! @brief ծառայողական բառերի ցուցակ
    static std::map<std::string_view,Token> keywords;

private:
    std::ifstream source; //!< նիշերը կարդալու հոսքը
    char ch = '\0'; //!< ընթացիկ նիշը

    unsigned int line = 1; //!< ընթացիկ տողը
};

} // basic

#endif
