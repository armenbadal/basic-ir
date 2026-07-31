#pragma once

#include <filesystem>

namespace basic {

// Կոմպիլյատորի ֆունկցիան
//
// source — Թարգմանվող ֆայլը
// generateIr — Գեներացնել IR կոդը
// generateLisp — Գեներացնել Lisp տեսքը
//
bool compile(const std::filesystem::path& source, bool generateIr, bool generateLisp);
} // basic

