
#include <iostream>
#include <string>

#include "../code/scanner.hxx"
#include "../code/tokens.hxx"

static const std::string testFile{"case0.bas"};

/**/
int main()
{
  Scanner sc{testFile};
  while( true ) {
    auto P = sc.line();
    auto T = sc.next();
    auto L = sc.lexeme();
    std::cout << P << " " << T << " " << L << std::endl;
    if( T == xEof ) break;
  }
}


