
#ifndef SYMTAB_HXX
#define SYMTAB_HXX

#include <list>
#include <string>
#include <utility>
#include <vector>

namespace basic {
/**/
using Symbol = std::pair<std::string,std::string>;

/**/
class SymbolTable {
private:
  using Scope = std::vector<Symbol>;

private:
  std::list<Scope> scopes;

public:
  SymbolTable() = default;
  ~SymbolTable();

  void openScope();
  void closeScope();

  void insert( const Symbol& );
  Symbol search( const std::string& );
};

} // basic

#endif

