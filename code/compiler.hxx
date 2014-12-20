
#ifndef COMPILER_HXX
#define COMPILER_HXX

#include <string>

unsigned int compile(const std::string&, bool = false);
void jitRun(const std::string&);

#endif

