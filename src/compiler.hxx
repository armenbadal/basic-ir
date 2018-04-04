
#ifndef COMPILER_HXX
#define COMPILER_HXX

#include <string>

namespace basic {
bool compile( const std::string& bas, bool ir = true, bool lisp = false );
} // basic

#endif // COMPILER_HXX

