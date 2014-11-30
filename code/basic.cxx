
#include <fstream>
#include <iostream>
#include <string>

#include "parser.hxx"

/**/
bool exists(const std::string& name)
{
  std::ifstream id{name};
  bool res{false};
  if( id.good() ) {
    id.close();
    res = true;
  }
  return res;
}

/**/
int main( int argc, char* argv[] )
{
  if( argc == 1 ) return 0;

  std::string basinp{argv[1]};
  if( !exists(basinp) ) {
    std::cerr << "'" + basinp + "' ֆայլի բացելու սխալ։" << std::endl;
    return 1;
  }

  std::string irout{basinp};
  auto p = irout.find_last_of(".");
  if( p != std::string::npos ) irout.erase(p);
  std::string lispout{irout};
  irout += ".ll";
  lispout += ".lisp";

  Parser sc{basinp};
  auto moduleAst = sc.parse();
  moduleAst->lisp(lispout);
  moduleAst->code(irout);

  delete moduleAst;
}

