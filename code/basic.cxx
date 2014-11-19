

#include <iostream>
#include <string>

#include "parser.hxx"


int main( int argc, char* argv[] )
{
  Parser sc{argv[1]};
  auto m = sc.parse();
  m->code("");
}

