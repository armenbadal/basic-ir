
#ifndef SYMTAB_H
#define SYMTAB_H

#include <string>
#include <vector>

/**/
enum class Scope : bool {
  Local = true,
  Global = false
};

/* -------------------------------------------------------------------- */
class Type {
public:
  virtual ~Type() {}
};
/**/
class Scalar : public Type {
public:
  std::string kind;
public:
  Scalar(const std::string& k) : kind{k} {}
};
/**/
class FuncType : public Type {
public:
  Type* rtype;
  std::vector<Type*> atypes;
public:
  FuncType(Type* r, std::vector<Type*>& a) : rtype{r}, atypes{a} {}
};

/* -------------------------------------------------------------------- */
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
  std::vector<Symbol*> items;
public:
  SymbolTable() {}
  void insert(Symbol*);
  Symbol* search(const std::string&);
};

#endif

