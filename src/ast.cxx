
#include "ast.hxx"

namespace basic {
  //
  Double::Double( double vl )
    : value{vl}
  {}

  //
  String::String( const std::string& vl )
    : value{vl}
  {}

  //
  Variable::Variable( const std::string& nm )
    : name{nm}
  {}

  //
  Unary::Unary( Operation op, Expression* ex )
    : opcode{op}, subexpr{ex}
  {}

  //
  Unary::~Unary()
  {
    delete subexpr;
  }
  
  //
  Binary::Binary( Operation op, Expression* exo, Expression* exi )
    : opcode{op}, subexpro{exo}, subexpri{exi}
  {}

  //
  Binary::~Binary()
  {
    delete subexpro;
    delete subexpri;
  }

  //
  Apply::Apply( const std::string& pn, const std::vector<Expression*>& ags )
    : procname{pn}, arguments{ags}
  {}

  //
  Apply::~Apply()
  {
    for( auto& ai : arguments )
      delete ai;
  }

  //
  Sequence::~Sequence()
  {
    for( auto& si : items )
      delete si;
  }
  
  //
  Input::Input( const std::string& vn )
    : varname{vn}
  {}

  //
  Print::Print( Expression* ex )
    : expr{ex}
  {}

  //
  Print::~Print()
  {
    delete expr;
  }

  //
  Let::Let( const std::string& vn, Expression* ex )
    : varname{vn}, expr{ex}
  {}

  //
  Let::~Let()
  {
    delete expr;
  }

  //
  If::If( Expression* co, Statement* de, Statement* al )
    : condition{co}, decision{de}, alternative{al}
  {}

  //
  If::~If()
  {
    delete condition;
    delete decision;
    delete alternative;
  }

  //
  While::While( Expression* co, Statement* bo )
    : condition{co}, body{bo}
  {}

  //
  While::~While()
  {
    delete condition;
    delete body;
  }

  //
  For( const std::string& pr, Expression* be, Expression* en, Expression* st, Statement* bo )
    : parameters{pr}, begin{be}, end{en}, step{st}, body{bo}
  {}
  
  //
  ~For()
  {}

  //
  Call::Call( const std::string& sn, const std::vector<Expression*> as )
    : subrcall{new Apply{sn, as}}
  {}

  //
  Call::~Call()
  {
    delete subrcall;
  }

  //
  Subroutine::Subroutine( const std::string& nm, const std::vector<std::string>& ps, Statement* bo )
    : name{nm}, parameters{ps}, body{bo}
  {}

  //
  Subroutine::~Subroutine()
  {
    delete body;
  }

  //
  Program::Program( const std::string& fn )
    : filename{fn}
  {}

  //
  Program::~Program()
  {
    for( auto& mi : members )
      delete mi;
  }
} // basic


