
#include "ast.hxx"

namespace basic {
  //
  std::list<AstNode*> AstNode::allocated_nodes;
  
  //
  void AstNode::delete_allocated_nodes()
  {
    for( auto e : allocated_nodes )
      delete e;
  }

  //
  AstNode::AstNode()
  {
    allocated_nodes.push_front(this);
  }
  
  //
  Number::Number( double vl )
    : value{vl}
  {
    type = Type::Number;
  }

  //
  Text::Text( const std::string& vl )
    : value{vl}
  {
    type = Type::Text;
  }

  //
  Variable::Variable( const std::string& nm )
    : name{nm}
  {
    type = name.back() == '$' ? Type::Text : Type::Number;
  }

  //
  Unary::Unary( Operation op, Expression* ex )
    : opcode{op}, subexpr{ex}
  {
    type = Type::Number;
  }

  //
  Binary::Binary( Operation op, Expression* exo, Expression* exi )
    : opcode{op}, subexpro{exo}, subexpri{exi}
  {}

  //
  Apply::Apply( const std::string& pn, const std::vector<Expression*>& ags )
    : procname{pn}, arguments{ags}
  {}

  //
  Input::Input( const std::string& vn )
    : varname{vn}
  {}

  //
  Print::Print( Expression* ex )
    : expr{ex}
  {}

  //
  Let::Let( const std::string& vn, Expression* ex )
    : varname{vn}, expr{ex}
  {}

  //
  If::If( Expression* co, Statement* de, Statement* al )
    : condition{co}, decision{de}, alternative{al}
  {}

  //
  While::While( Expression* co, Statement* bo )
    : condition{co}, body{bo}
  {}

  //
  For::For( const std::string& pr, Expression* be, Expression* en, Expression* st, Statement* bo )
    : parameter{pr}, begin{be}, end{en}, step{st}, body{bo}
  {}
  
  //
  Call::Call( const std::string& sn, const std::vector<Expression*> as )
    : subrcall{new Apply{sn, as}}
  {}

  //
  Subroutine::Subroutine( const std::string& nm, const std::vector<std::string>& ps, Statement* bo )
    : name{nm}, parameters{ps}, body{bo}
  {}

  //
  Program::Program( const std::string& fn )
    : filename{fn}
  {}
} // basic


