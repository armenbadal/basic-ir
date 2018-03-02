
#include <fstream>
#include <iostream>
#include <sstream>

#include "llvm/Support/FileSystem.h"

#include "asdot.hxx"
#include "aslisp.hxx"
#include "parser.hxx"
#include "scanner.hxx"

//
int main()
{
    std::cout << "Parsing ..." << std::endl;

    basic::Parser parser("../cases/case04.bas");
    basic::Program* prog = nullptr;
    try {
        prog = parser.parse();
    }
    catch (basic::ParseError& e) {
        std::cerr << e.what() << std::endl;
    }
    catch (basic::TypeError& e) {
        std::cerr << e.what() << std::endl;
    }

    if (nullptr != prog) {
        std::cout << "Lisp output..." << std::endl;
        basic::Lisper(std::cout).convert(prog);
        std::cout << std::endl
                  << std::endl;

        std::cout << "DOT output..." << std::endl;
        basic::Doter(std::cout).convert(prog);
    }

    /*
  std::cout << "End Parsing ..." << std::endl;
  if( nullptr != prog ) {
    std::ostringstream out;
    std::error_code ec;
    llvm::raw_fd_ostream ef("emitted.ll", ec, llvm::sys::fs::F_RW);
    std::cout << "Compiling ..." << std::endl;
    basic::emitLLVM(prog, ef);
    std::cout << out.str() << std::endl;
    basic::AstNode::deleteAllocatedNodes();
  }
  */

    return 0;
}
