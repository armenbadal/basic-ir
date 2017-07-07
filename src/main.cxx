
#include <iostream>
#include <sstream>

#include "parser.hxx"
#include "scanner.hxx"

namespace basic {
  void lisp( AstNode* node, std::ostringstream& ooo );
}

//
int main()
{
  basic::Parser parser("../cases/case00.bas");
  auto prog = parser.parse();

  if( nullptr != prog ) {
    std::ostringstream out;
    basic::lisp(prog, out);
    std::cout << out.str() << std::endl;
    basic::AstNode::delete_allocated_nodes();
  }
  
  return 0;
}


