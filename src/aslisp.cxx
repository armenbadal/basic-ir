
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
  void lisp( AstNode* node, std::ostringstream& ooo );
  
  ///
  void numberAsLisp( Number* node, std::ostringstream& ooo )
  {
    ooo << "(basic-number :value " << node->value << ")";
  }
  
  ///
  void textAsLisp( Text* node, std::ostringstream& ooo )
  {
    ooo << "(basic-text :value \"" << node->value << "\")";
  }
  
  ///
  void variableAsLisp( Variable* node, std::ostringstream& ooo )
  {
    ooo << "(basic-variable :name \"" << node->name << "\")";
  }

  ///
  void unaryAsLisp( Unary* node, std::ostringstream& ooo )
  {
    ooo << "(basic-unary :opcode \"" << mnemonic[node->opcode] << "\" :subexpr ";
    lisp(node->subexpr, ooo);
    ooo << ")";
  }

  ///
  void binaryAsLisp( Binary* node, std::ostringstream& ooo )
  {
    ooo << "(basic-binary :opcode \"" << mnemonic[node->opcode] << "\"";
    ooo << " :subexpro ";
    lisp(node->subexpro, ooo);
    ooo << " :subexpri ";
    lisp(node->subexpri, ooo);
    ooo << ")";
  }

  ///
  void applyAsLisp( Apply* node, std::ostringstream& ooo )
  {
    ooo << "(basic-apply :procname \"" << node->procname << "\"";
    ooo << " :arguments (";
    for( auto e : node->arguments ) {
      lisp(e, ooo);
      ooo << " ";
    }
    ooo << "))";
  }

  ///
  void letAsLisp( Let* node, std::ostringstream& ooo )
  {
    ooo << "(basic-let :varname " << node->varname << " :expr ";
    lisp(node->expr, ooo);
    ooo << ")";
  }

  ///
  void inputAsLisp( Input* node, std::ostringstream& ooo )
  {
    ooo << "(basic-input :varname " << node->varname << ")";
  }

  ///
  void printAsLisp( Print* node, std::ostringstream& ooo )
  {
    ooo << "(basic-print :expr ";
    lisp(node->expr, ooo);
    ooo << ")";
  }

  ///
  void ifAsLisp( If* node, std::ostringstream& ooo )
  {
    ooo << "(basic-if :condition ";
    lisp(node->condition, ooo);
    ooo << " :decision ";
    lisp(node->decision, ooo);
    ooo << " :alternative ";
    lisp(node->alternative, ooo);
    ooo << ")";
  }

  ///
  void whileAsLisp( While* node, std::ostringstream& ooo )
  {
    ooo << "(basic-while :condition ";
    lisp(node->condition, ooo);
    ooo << " :body ";
    lisp(node->body, ooo);
    ooo << ")";
  }

  ///
  void forAsLisp( For* node, std::ostringstream& ooo )
  {
    ooo << "(basic-for :parameter " << node->parameter;
    ooo << " :begin ";
    lisp(node->begin, ooo);
    ooo << " :end ";
    lisp(node->end, ooo);
    ooo << " :step ";
    lisp(node->step, ooo);
    ooo << " :body ";
    lisp(node->body, ooo);
    ooo << ")";
  }

  ///
  void callAsLisp( Call* node, std::ostringstream& ooo )
  {
    ooo << "(basic-call :procname " << node->subrcall->procname;
    ooo << " :arguments (";
    for( auto e : node->subrcall->arguments ) {
      lisp(e, ooo);
      ooo << " ";
    }
    ooo << "))";
  }

  ///
  void sequenceAsLisp( Sequence* node, std::ostringstream& ooo )
  {
    ooo << "(basic-sequence :items (";
    for( auto ei : node->items ) {
      lisp(ei, ooo);
      ooo << " ";
    }
    ooo << "))";
  }

  ///
  void subroutineAsLisp( Subroutine* node, std::ostringstream& ooo )
  {
    ooo << "(basic-subroutine :name " << node->name;
    ooo << " :parameters (";
    for( auto& ip : node->parameters )
      ooo << ip << " ";
    ooo << ") :body ";
    lisp(node->body, ooo);
    ooo << ")";
  }

  ///
  void programAsLisp( Program* node, std::ostringstream& ooo )
  {
    ooo << "(basic-program :filename " << node->filename;
    ooo << " :members (";
    for( auto si : node->members ) {
      ooo << "\n";
      lisp(si, ooo);
    }
    ooo << "))\n";
  }

  ///
  void lisp( AstNode* node, std::ostringstream& ooo )
  {
    switch( node->kind ) {
      case NodeKind::Number:
        numberAsLisp(dynamic_cast<Number*>(node), ooo);
        break;
      case NodeKind::Text:
	textAsLisp(dynamic_cast<Text*>(node), ooo);
	break;
      case NodeKind::Variable:
	variableAsLisp(dynamic_cast<Variable*>(node), ooo);
	break;
      case NodeKind::Unary:
	unaryAsLisp(dynamic_cast<Unary*>(node), ooo);
	break;
      case NodeKind::Binary:
	binaryAsLisp(dynamic_cast<Binary*>(node), ooo);
	break;
      case NodeKind::Apply:
	applyAsLisp(dynamic_cast<Apply*>(node), ooo);
	break;
      case NodeKind::Sequence:
	sequenceAsLisp(dynamic_cast<Sequence*>(node), ooo);
	break;
      case NodeKind::Input:
	inputAsLisp(dynamic_cast<Input*>(node), ooo);
	break;
      case NodeKind::Print:
	printAsLisp(dynamic_cast<Print*>(node), ooo);
	break;
      case NodeKind::Let:
	letAsLisp(dynamic_cast<Let*>(node), ooo);
	break;
      case NodeKind::If:
	ifAsLisp(dynamic_cast<If*>(node), ooo);
	break;
      case NodeKind::While:
	whileAsLisp(dynamic_cast<While*>(node), ooo);
	break;
      case NodeKind::For:
	forAsLisp(dynamic_cast<For*>(node), ooo);
	break;
      case NodeKind::Call:
	callAsLisp(dynamic_cast<Call*>(node), ooo);
	break;
      case NodeKind::Subroutine:
	subroutineAsLisp(dynamic_cast<Subroutine*>(node), ooo);
	break;
      case NodeKind::Program:
	programAsLisp(dynamic_cast<Program*>(node), ooo);
	break;
      default:
	{}
    }
  }
} // basic

