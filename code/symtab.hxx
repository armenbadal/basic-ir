
#ifndef SYMTAB_H
#define SYMTAB_H

#include <list>
#include <string>
#include <utility>
#include <vector>

/* -------------------------------------------------------------------- */
class Type {
public:
  virtual ~Type() {}
  virtual bool operator==(const Type&) = 0;
  virtual bool operator!=(const Type&) = 0;
  virtual std::string name() = 0;
};

/**/
enum class TypeID : unsigned char {
  Void = 'V',
  Character = 'C',
  Integer = 'I',
  Double = 'D',
  Boolean = 'B',
  String = 'S'
};

/**/
class Scalar : public Type {
public:
  TypeID kind;

public:
  Scalar(TypeID k) : kind{k} {}
  Scalar(const std::string&);
  bool operator==(const Type&);
  bool operator!=(const Type&);
};

/**/
class FuncType : public Type {
public:
  Type* rtype;
  std::vector<Type*> atypes;

public:
  FuncType(Type* r, std::vector<Type*>& a) 
    : rtype{r}, atypes{a} {}
  ~FuncType();
  bool operator==(const Type&);
  bool operator!=(const Type&);
};

/* -------------------------------------------------------------------- */
using Symbol = std::pair<std::string,Type*>;

std::string nameOf(Symbol*);
Type* typeOf(Symbol*);

/**/
class SymbolTable {
private:
  using Scope = std::vector<Symbol*>;

private:
  std::list<Scope> scopes;

public:
  SymbolTable() {}
  ~SymbolTable();

  void openScope();
  void closeScope();

  void insert(Symbol*);
  Symbol* search(const std::string&);

  void dump(); // DEBUG
};

#endif

