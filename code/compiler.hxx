
#ifndef COMPILER_HXX
#define COMPILER_HXX

#include <string>

class Module;

/**/
Module* compile(const std::string&, bool = false);
void generate(Module*);
void jitRun(Module*);

#endif

