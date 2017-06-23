
#include <iostream>
#include <sstream>

#include "parser.hxx"
#include "scanner.hxx"

//
int main()
{
  basic::Parser parser("../cases/case01.bas");
  auto prog = parser.parseProgram();

  std::ostringstream out;
  prog->lisp(out);
  std::cout << out.str() << std::endl;
  
  return 0;
}


