
#ifndef COMPILER_HXX
#define COMPILER_HXX

#include <string>

namespace llvm {
  class Module;
}

/**/
class Compiler {
public:
  static void compile(const std::string&, bool = false);
};

#endif

