
#include <iostream> /* DEBUG */
#include <algorithm>

#include "symtab.hxx"

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
  auto pr = [&nm](Symbol s)->bool{ return nm == s.first; };
  for( auto& sc : scopes ) {
    auto it = std::find_if( sc.begin(), sc.end(), pr );
    if( it != sc.end() ) return *it;
  }
  return Symbol("","");
}

/* --- DEBUG --- */
void SymbolTable::dump()
{
  std::cout << std::string(20, '~') << std::endl;
  int level = scopes.size();
  for( auto& sc : scopes ) {
    std::string indent(--level, ' ');
    for( auto e : sc )
      std::cout << indent << e.first << " : " << e.second << std::endl;
  }
}


