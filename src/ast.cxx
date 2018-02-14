
#include "ast.hxx"
#include <iostream>

namespace basic {
  //
  std::list<AstNode*> AstNode::allocated_nodes;
  
  //
  void AstNode::delete_allocated_nodes()
  {
    for( auto e : allocated_nodes )
      delete e;
  }

  void AstNode::printKind()
  {
      switch (kind) {
          case NodeKind::Empty:
              std::cout << "Empty" << std::endl;
              break;
          case NodeKind::Number:
              std::cout << "Number:" << std::endl;
              break;
          case NodeKind::Text:
              std::cout << "Text:" << std::endl;
              break;
          case NodeKind::Variable:
              std::cout << "Variable:" << std::endl;
              break;
          case NodeKind::Unary:
              std::cout << "Unary:" << std::endl;
              break;
          case NodeKind::Binary:
              std::cout << "Binary:" << std::endl;
              break;
          case NodeKind::Apply:
              std::cout << "Apply" << std::endl;
              break;
          case NodeKind::Sequence:
              std::cout << "Sequence" << std::endl;
              break;
          case NodeKind::Input:
              std::cout << "Input" << std::endl;
              break;
          case NodeKind::Print:
              std::cout << "Print" << std::endl;
              break;
          case NodeKind::Let:
              std::cout << "Let" << std::endl;
              break;
          case NodeKind::If:
              std::cout << "If" << std::endl;
              break;
          case NodeKind::While:
              std::cout << "While" << std::endl;
              break;
          case NodeKind::For:
              std::cout << "For" << std::endl;
              break;
          case NodeKind::Call:
              std::cout << "Call" << std::endl;
              break;
          case NodeKind::Subroutine:
              std::cout << "Subroutine" << std::endl;
              break;
          case NodeKind::Program:
              std::cout << "Program" << std::endl;
              break;
          default : 
              std::cout << "UNKNOWN" << std::endl;

      }
  }

  //
  AstNode::AstNode()
  {
    allocated_nodes.push_front(this);
  }
  
  Sequence::Sequence()
  {
    kind = NodeKind::Sequence;
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
    locals.push_back(new Variable(name));
  }

  //
  Program::Program( const std::string& fn )
    : filename{fn}
  {
    kind = NodeKind::Program;
  }
} // basic


