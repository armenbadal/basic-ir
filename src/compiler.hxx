#pragma once

#include <filesystem>

namespace basic {

//! @brief Կոմպիլյատորի ֆունկցիան
//!
//! @param source Թարգմանվող ֆայլը
//! @param generateIr Գեներացնել IR կոդը
//! @param generateLisp Գեներացնել Lisp տեսքը
//! 
bool compile(const std::filesystem::path& source, bool generateIr, bool generateLisp);
} // basic

