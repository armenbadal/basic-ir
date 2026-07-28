#pragma once

#include <filesystem>
#include <fstream>
#include <istream>
#include <map>
#include <string_view>
#include <sstream>
#include <memory>
#include <stdexcept>

#include "lexeme.hxx"

namespace basic {

//! @brief Բառային վերլուծիչ
class Scanner {
public:
    explicit Scanner(std::istream& source);

    Lexeme scan();

private:
    Lexeme scanNumber();
    Lexeme scanText();
    Lexeme scanIdentifier();

    std::istream& source;
    unsigned int line = 1;
};

} // basic
