
#ifndef SCANNER_HXX
#define SCANNER_HXX

#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <string>

#include "lexeme.hxx"

namespace basic {
//! @brief Բառային վերլուծիչ
class Scanner {
public:
    //! @brief Բառային վերլուծիչի կոնստրուկտոր
    //!
    //! @param filename Վերլուծվելիք ֆայլի ճանապարհը
    Scanner(const std::filesystem::path& filename);

    //! @brief Բառային վերլուծիչի դեստրուկտոր
    ~Scanner();

    //! @brief Հերթական լեքսեմը կարդալու օպերատոր
    Scanner& operator>>(Lexeme& lex);

private:
    //! @brief Հերթական լեքսեմը կարդալու ֆունկցիա
    bool next(Lexeme& lex);

    //! @brief Թվային լիտերալ
    bool scanNumber(Lexeme& lex);

    //! @brief Տեքստային լիտերալ
    bool scanText(Lexeme& lex);

    //! @brief Իդենտիֆիկատոր
    bool scanIdentifier(Lexeme& lex);

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
