
#include "compiler.hxx"

#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    // basic::compile("../cases/case01.bas", true, true);
    if( argc > 1 )
        basic::compile(argv[1], true, true);

    return 0;
}
