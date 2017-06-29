
#include <iostream>
#include <sstream>

#include "parser.hxx"
#include "scanner.hxx"

//
int main()
{
  basic::Parser parser("../cases/case03.bas");
  auto prog = parser.parse();

  if( nullptr != prog ) {
    std::ostringstream out;
    prog->lisp(out);
    std::cout << out.str() << std::endl;
    basic::AstNode::delete_allocated_nodes();
  }
  
  return 0;
}


