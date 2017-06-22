
#include "parser.hxx"
#include "scanner.hxx"

//
int main()
{
  basic::Scanner scanner("../cases/case00.bas");
  basic::Parser parser(scanner);

  parser.parseProgram();
  
  return 0;
}


