
#include <iostream>

#include "scanner.hxx"


int main( int argc, char* argv[] )
{
  Scanner sc{argv[1]};
  Token tok{xNull};
  while( (tok = sc.next()) != xEof ) 
    //std::cout << tok << '|' << sc.lexeme() << std::endl;
    ;
}

