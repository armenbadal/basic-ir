
#include "ast.hxx"

#include <map>
#include <string>

namespace basic {
  ///
  std::map<Operation,std::string> mnemonic{
    {Operation::Add, "ADD"},
    {Operation::Sub, "SUB"},
    {Operation::Conc, "CONC"},
    {Operation::Mul, "MUL"},
    {Operation::Div, "DIV"},
    {Operation::Mod, "MOD"},
    {Operation::Pow, "POW"},
    {Operation::Eq, "EQ"},
    {Operation::Ne, "NE"},
    {Operation::Gt, "GT"},
    {Operation::Ge, "GE"},
    {Operation::Lt, "LT"},
    {Operation::Le, "LE"},
    {Operation::And, "AND"},
    {Operation::Or, "OR"}
  };
  
  ///
  void Number::lisp( std::ostringstream& ooo )
  {
    ooo << "(basic-number :value " << value << ")";
  }
  
  ///
  void Text::lisp( std::ostringstream& ooo )
  {
    ooo << "(basic-text :value \"" << value << "\")";
  }
  
  ///
  void Variable::lisp( std::ostringstream& ooo )
  {
    ooo << "(basic-variable :name \"" << name << "\")";
  }

  ///
  void Unary::lisp( std::ostringstream& ooo )
  {
    ooo << "(basic-unary :opcode \"" << mnemonic[opcode] << "\" :subexpr ";
    subexpr->lisp(ooo);
    ooo << ")";
  }

  ///
  void Binary::lisp( std::ostringstream& ooo )
  {
    ooo << "(basic-binary :opcode \"" << mnemonic[opcode] << "\"";
    ooo << " :subexpro "; subexpro->lisp(ooo);
    ooo << " :subexpri "; subexpri->lisp(ooo);
    ooo << ")";
  }

  ///
  void Apply::lisp( std::ostringstream& ooo )
  {
    ooo << "(basic-apply :procname \"" << procname << "\"";
    ooo << " :arguments (";
    for( auto e : arguments ) {
      e->lisp(ooo);
      ooo << " ";
    }
    ooo << "))";
  }

  ///
  void Let::lisp( std::ostringstream& ooo )
  {
    ooo << "(basic-let :varname " << varname << " :expr ";
    expr->lisp(ooo);
    ooo << ")";
  }

  ///
  void Input::lisp( std::ostringstream& ooo )
  {
    ooo << "(basic-input :varname " << varname << ")";
  }

  ///
  void Print::lisp( std::ostringstream& ooo )
  {
    ooo << "(basic-print :expr ";
    expr->lisp(ooo);
    ooo << ")";
  }

  ///
  void If::lisp( std::ostringstream& ooo )
  {
    ooo << "(basic-if :condition ";
    condition->lisp(ooo);
    ooo << " :decision ";
    decision->lisp(ooo);
    ooo << " :alternative ";
    alternative->lisp(ooo);
    ooo << ")";
  }

  ///
  void While::lisp( std::ostringstream& ooo )
  {
    ooo << "(basic-while :condition ";
    condition->lisp(ooo);
    ooo << " :body ";
    body->lisp(ooo);
    ooo << ")";
  }

  ///
  void For::lisp( std::ostringstream& ooo )
  {
    ooo << "(basic-for :parameter " << parameter;
    ooo << " :begin ";
    begin->lisp(ooo);
    ooo << " :end ";
    end->lisp(ooo);
    ooo << " :step ";
    step->lisp(ooo);
    ooo << " :body ";
    body->lisp(ooo);
    ooo << ")";
  }

  ///
  void Call::lisp( std::ostringstream& ooo )
  {
    ooo << "(basic-call :procname " << subrcall->procname;
    ooo << " :arguments (";
    for( auto e : subrcall->arguments ) {
      e->lisp(ooo); ooo << " ";
    }
    ooo << "))";
  }

  ///
  void Sequence::lisp( std::ostringstream& ooo )
  {
    ooo << "(basic-sequence :items (";
    for( auto ei : items ) {
      ei->lisp(ooo); ooo << " ";
    }
    ooo << "))";
  }

  ///
  void Subroutine::lisp( std::ostringstream& ooo )
  {
    ooo << "(basic-subroutine :name " << name;
    ooo << " :parameters (";
    for( auto& ip : parameters )
      ooo << ip << " ";
    ooo << ") :body ";
    body->lisp(ooo);
    ooo << ")";
  }

  ///
  void Program::lisp( std::ostringstream& ooo )
  {
    ooo << "(basic-program :filename " << filename;
    ooo << " :members (";
    for( auto si : members ) {
      ooo << "\n";
      si->lisp(ooo);
    }
    ooo << "))\n";
  }
} // basic

