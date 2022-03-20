
#ifndef COMPILER_HXX
#define COMPILER_HXX

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

#endif // COMPILER_HXX

