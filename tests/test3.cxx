
#include <iostream>

#include "../code/symtab.hxx"


/**/
int main()
{
  SymbolTable st;

  st.openScope();
  st.insert( new Symbol("A", nullptr) );
  st.insert( new Symbol("B", nullptr) );
  st.insert( new Symbol("C", nullptr) );

  st.openScope();
  st.insert( new Symbol("D", nullptr) );
  st.insert( new Symbol("E", nullptr) );

  st.openScope();
  st.insert( new Symbol("F", nullptr) );
  st.insert( new Symbol("G", nullptr) );

  st.dump();

  st.closeScope();
  st.insert( new Symbol("H", nullptr) );
  st.insert( new Symbol("I", nullptr) );

  st.dump();

  auto el = st.search( "A" );
  std::cout << nameOf(el) << std::endl;

  el = st.search( "D" );
  std::cout << nameOf(el) << std::endl;

  el = st.search( "G" );
  std::cout << (el == nullptr) << std::endl;

  st.closeScope();
  st.closeScope();
  st.closeScope();
}


