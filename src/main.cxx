
#include <iostream>
#include <sstream>
#include <fstream>

#include "llvm/Support/FileSystem.h"

#include "parser.hxx"
#include "scanner.hxx"


namespace basic {
  void lisp( AstNode* node, std::ostringstream& ooo );
  void emitLLVM( Program* node, llvm::raw_fd_ostream& ooo );
}

//
int main()
{
  std::cout << "Parsing ..." << std::endl;
  basic::Parser parser("../cases/case05.bas");
  auto prog = parser.parse();

  std::cout << "End Parsing ..." << std::endl;
  if( nullptr != prog ) {
    std::ostringstream out;
    std::error_code ec;
    llvm::raw_fd_ostream ef("emitted.ll", ec, llvm::sys::fs::F_RW);
    std::cout << "Compiling ..." << std::endl;
    basic::emitLLVM(prog, ef);
    std::cout << out.str() << std::endl;
    basic::AstNode::delete_allocated_nodes();
  }
  
  return 0;
}


