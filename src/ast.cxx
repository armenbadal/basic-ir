
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
    kind = NodeKind::Number;
    type = Type::Number;
  }

  //
  Text::Text( const std::string& vl )
    : value{vl}
  {
    kind = NodeKind::Text;
    type = Type::Text;
  }

  //
  Variable::Variable( const std::string& nm )
    : name{nm}
  {
    kind = NodeKind::Variable;
    type = name.back() == '$' ? Type::Text : Type::Number;
  }

  //
  Unary::Unary( Operation op, Expression* ex )
    : opcode{op}, subexpr{ex}
  {
    kind = NodeKind::Unary;
    type = Type::Number;
  }

  //
  Binary::Binary( Operation op, Expression* exo, Expression* exi )
    : opcode{op}, subexpro{exo}, subexpri{exi}
  {
    kind = NodeKind::Binary;
  }

  //
  Apply::Apply( const std::string& pn, const std::vector<Expression*>& ags )
    : procname{pn}, arguments{ags}
  {
    kind = NodeKind::Apply;
  }

  //
  Input::Input( const std::string& vn )
    : varname{vn}
  {
    kind = NodeKind::Input;
  }

  //
  Print::Print( Expression* ex )
    : expr{ex}
  {
    kind = NodeKind::Print;
  }

  //
  Let::Let( const std::string& vn, Expression* ex )
    : varname{vn}, expr{ex}
  {
    kind = NodeKind::Let;
  }

  //
  If::If( Expression* co, Statement* de, Statement* al )
    : condition{co}, decision{de}, alternative{al}
  {
    kind = NodeKind::If;
  }

  //
  While::While( Expression* co, Statement* bo )
    : condition{co}, body{bo}
  {
    kind = NodeKind::While;
  }

  //
  For::For( const std::string& pr, Expression* be, Expression* en, Expression* st, Statement* bo )
    : parameter{pr}, begin{be}, end{en}, step{st}, body{bo}
  {
    kind = NodeKind::For;
  }
  
  //
  Call::Call( const std::string& sn, const std::vector<Expression*> as )
    : subrcall{new Apply{sn, as}}
  {
    kind = NodeKind::Call;
  }

  //
  Subroutine::Subroutine( const std::string& nm, const std::vector<std::string>& ps, Statement* bo )
    : name{nm}, parameters{ps}, body{bo}
  {
    kind = NodeKind::Subroutine;
  }

  //
  Program::Program( const std::string& fn )
    : filename{fn}
  {
    kind = NodeKind::Program;
  }
} // basic


