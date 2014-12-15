
#include <iostream>

#include "../code/symtab.hxx"

/**/
class P {
public:
  SymbolTable s;
};

/**/
int main()
{
  P p;
  p.s.openScope();

  p.s.insert( Symbol{"A", "B"} );
  p.s.insert( Symbol{"C", "D"} );

  auto a = p.s.search("A");
  std::cout << a.first << " " << a.second << std::endl;

  a = p.s.search("C");
  std::cout << a.first << " " << a.second << std::endl;

  p.s.closeScope();
}


