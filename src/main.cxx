
#include <iostream>
#include <sstream>

#include "parser.hxx"
#include "scanner.hxx"

namespace basic {
  void lisp( AstNode* node, std::ostringstream& ooo );
  void programAsLLVM( Program* node, std::ostringstream& ooo );
}

//
int main()
{
    std::cout << "Parsing ..." << std::endl;
  basic::Parser parser("../cases/case01.bas");
  auto prog = parser.parse();

    std::cout << "End Parsing ..." << std::endl;
  if( nullptr != prog ) {
    std::ostringstream out;
    std::cout << "Lisping ..." << std::endl;
    //basic::lisp(prog, out);
    basic::programAsLLVM(prog, out);
    std::cout << out.str() << std::endl;
    basic::AstNode::delete_allocated_nodes();
  }
  
  return 0;
}


