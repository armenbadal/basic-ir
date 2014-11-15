
#ifndef SYMTAB_H
#define SYMTAB_H

#include <list>
#include <string>

/**/
enum class Scope : bool {
  Local = true,
  Global = false
};

/**/
class Symbol {
public:
  Scope scope;
  std::string name;
  std::string type;
public:
  Symbol(Scope sc, const std::string& nm, const std::string& ty)
    : scope{sc}, name{nm}, type{ty}
  {}
  
};

/**/
class SymbolTable {
private:
  SymbolTable* top;
  std::list<Symbol*> items;
public:
  SymbolTable() {}
  void insert(Symbol*);
  Symbol* search(const std::string&);
};

#endif

