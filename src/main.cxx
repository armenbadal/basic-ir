
#include <fstream>
#include <iostream>
#include <sstream>

#include <llvm/Support/FileSystem.h>

#include "aslisp.hxx"
#include "parser.hxx"
#include "scanner.hxx"
#include "iremitter.hxx"

bool fileExists(const std::string& filename)
{
    std::ifstream infi(filename);
    return infi.good();
}

//
int main( int argc, char* argv[] )
{
    std::cout << "Parsing ..." << std::endl;

    basic::Parser parser("../cases/case09.bas");
    auto prog = parser.parse();

    if( argc < 2 ) {
        std::cout << "" << std::endl;
        return 0;
    }

    if( !fileExists(argv[1]) ) {
        std::cout << argv[1] << " ֆայլը գոյություն չունի։";
        return 0;
    }

    std::cout << "Parsing ..." << std::endl;
    basic::Parser parser(argv[1]);
    auto prog = parser.parse();

    if( nullptr != prog ) {
        std::cout << "Type checking ..." << std::endl;
        bool errok = basic::TypeChecker().check(prog);

        if( errok ) {
            std::cout << "Lisp ..." << std::endl;
            //std::ofstream sout(std::string(argv[1]) + ".lisp");
            basic::Lisper(std::cout).asLisp(prog);
            //sout.close();
        }
    }

    return 0;
}
