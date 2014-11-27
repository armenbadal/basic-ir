
#include <fstream>
#include <iostream>
#include <string>

#include "parser.hxx"

/**/
int main( int argc, char* argv[] )
{
  Parser sc{argv[1]};
  auto m = sc.parse();
  m->code("");

  /* DEBUG */
  std::ofstream oast{"ast.lisp"};
  oast << "(load \"astm.lisp\")" << std::endl;
  oast << "(pprint ";
  m->lisp(oast);
  oast << ")" << std::endl << "(terpri)(quit)" << std::endl;
  oast.close();
}

