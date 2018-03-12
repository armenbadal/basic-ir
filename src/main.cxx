
#include <fstream>
#include <iostream>
#include <sstream>

#include <llvm/Support/FileSystem.h>

#include "aslisp.hxx"
#include "parser.hxx"
#include "scanner.hxx"
#include "iremitter.hxx"

//
int main()
{
    std::cout << "Parsing ..." << std::endl;

    basic::Parser parser("../cases/case09.bas");
    auto prog = parser.parse();

    if( nullptr != prog ) {
        std::cout << "Lisp output..." << std::endl;
        basic::Lisper(std::cout).asLisp(prog);
        std::cout << std::endl;
    }

    if( nullptr != prog ) {
        std::error_code ec;
        llvm::raw_fd_ostream ef("emitted.ll", ec, llvm::sys::fs::F_RW);
        std::cout << "Compiling ..." << std::endl;
        basic::IrEmitter(ef).emitIrCode(prog);
    }

    return 0;
}
