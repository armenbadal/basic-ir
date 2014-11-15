
#include <algorithm>

#include "symtab.hxx"

/**/
void SymbolTable::insert(Symbol* sp)
{
  items.push_back( sp );
}

/**/
Symbol* SymbolTable::search(const std::string& nm)
{
  auto pr = [nm](Symbol s)->bool{ return s->name == nm; }
  auto it = std::find(items.begin(), items.end(), pr);
  if( it != items.end() ) return *it;
  return nullptr;
}

