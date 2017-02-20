
#include <algorithm>

#include "symtab.hxx"

namespace basic {
/**/
SymbolTable::~SymbolTable()
{
  while( !scopes.empty() )
    closeScope();
}

/**/
void SymbolTable::openScope()
{
  scopes.push_front( Scope{} );
}

/**/
void SymbolTable::closeScope()
{
  if( !scopes.empty() )
    scopes.pop_front();
}

/**/
void SymbolTable::insert(const Symbol& sp)
{
  scopes.front().push_back( sp );
}

/**/
Symbol SymbolTable::search(const std::string& nm)
{
  auto pr = [&nm](Symbol s)->bool { return nm == s.first; };
  for( auto& sc : scopes ) {
    auto it = std::find_if( sc.begin(), sc.end(), pr );
    if( it != sc.end() )
      return *it;
  }
  return Symbol{"",""};
}

} // basic

