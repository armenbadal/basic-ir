
#include <iostream>

#include "parser.hxx"


int main( int argc, char* argv[] )
{
  Parser sc{argv[1]};
  sc.parse();
}

