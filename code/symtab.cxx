
#include <algorithm>
#include <typeinfo>

#include "symtab.hxx"

/**/
Scalar::Scalar(const std::string& id)
{
  if( id == "Integer" )
    kind = TypeID::Integer;
  else if( id == "Double" )
    kind = TypeID::Double;
  else if( id == "Boolean" )
    kind = TypeID::Boolean;
  kind = TypeID::Void;
}

/**/
bool Scalar::operator==(const Type& other)
{
  if( typeid(*this) != typeid(other) ) return false;
  return kind == dynamic_cast<const Scalar&>(other).kind; 
}

/**/
bool Scalar::operator!=(const Type& other)
{
  return !(*this == other);
}

/**/
FuncType::~FuncType()
{
  delete rtype;
  for( auto e : atypes )
    delete e;
}

/**/
bool FuncType::operator==(const Type& other)
{
  if( typeid(*this) != typeid(other) ) return false;
  const FuncType& ur = dynamic_cast<const FuncType&>(other);
  if( *rtype != *(ur.rtype) )  return false;
  if( atypes.size() != ur.atypes.size() ) return false;
  for( int i = 0; i < atypes.size(); ++i )
    if( *atypes[i] != *(ur.atypes[i]) ) return false;
  return true;
}

/**/
bool FuncType::operator!=(const Type& other)
{
  return !(*this == other);
}

/**/
std::string nameOf(Symbol* s)
{
  return s->first;
}
/**/
Type* typeOf(Symbol* s)
{
  return s->second;
}

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
  if( !scopes.empty() ) {
    for( auto e : scopes.front() )
      delete e;
    scopes.pop_front();
  }
}

/**/
void SymbolTable::insert(Symbol* sp)
{
  scopes.front().push_back( sp );
}

/**/
Symbol* SymbolTable::search(const std::string& nm)
{
  auto pr = [&nm](Symbol* s)->bool{ return nm == nameOf(s); };
  for( auto& sc : scopes ) {
    auto it = std::find_if( sc.begin(), sc.end(), pr );
    if( it != sc.end() ) return *it;
  }
  return nullptr;
}

/* --- DEBUG --- */
#include <iostream>

/**/
void SymbolTable::dump()
{
  std::cout << std::string(20, '~') << std::endl;
  int level = scopes.size();
  for( auto& sc : scopes ) {
    std::string indent(--level, ' ');
    for( auto e : sc )
      std::cout << indent << nameOf(e) << std::endl;
  }
}


