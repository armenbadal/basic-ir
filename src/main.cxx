
#include "compiler.hxx"

#include <iostream>


int main(int argc, char* argv[])
{
    basic::compile("../cases/case18.bas", true, true);
    //if( argc > 1 )
    //    std::cout << basic::compile(argv[1], true, true) << std::endl;

    return 0;
}
