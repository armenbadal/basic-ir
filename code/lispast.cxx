
#include <fstream>

#include "ast.hxx"

/**/
void Module::lisp(const std::string& eman)
{
  std::ofstream ooo{eman};
  ooo << "(defconstant +abstract-syntax-tree+ ";
  ooo << "#S(ast-module";
  ooo << " :name \"" << name << "\"";
  ooo << " :subs '(";
  for( auto f : subs ) f->lisp(ooo); 
  ooo << ")))" << std::endl;
  ooo.close();
}

/**/
void Function::lisp(std::ostream& ooo)
{
  ooo << "#S(ast-function";
  ooo << " :name \"" << name;
  ooo << "\" :args '(";
  for( auto& a : args )
    ooo << "(\"" << a.first << "\" . \"" << a.second << "\")";
  ooo << ") :type \"" << type << "\" ";
  ooo << ":body ";
  body->lisp(ooo);
  ooo << ")";
}

/**/
void Variable::lisp(std::ostream& ooo)
{
  ooo << "#S(ast-variable :name \"" << name;
  ooo << "\" :type \"" << type << "\")";
}

/**/
void Integer::lisp(std::ostream& ooo)
{
  ooo << "#S(ast-constant :value " << value;
  ooo << " :type \"Integer\")";
}

/**/
void Double::lisp(std::ostream& ooo)
{
  ooo << "#S(ast-constant :value \"" << value;
  ooo << "\" :type \"Double\")";
}

/**/
void Boolean::lisp(std::ostream& ooo)
{
  std::string sv{value ? "\"True\"" : "\"False\""};
  ooo << "#S(ast-constant :value " << sv << " :type \"Boolean\")";
}

/**/
void Unary::lisp(std::ostream& ooo)
{
  ooo << "#S(ast-unary";
  ooo << " :oper " << oper;
  ooo << " :expr ";
  expr->lisp(ooo);
  ooo << ")";
}

/**/
void TypeCast::lisp(std::ostream& ooo)
{
  ooo << "#S(ast-type-cast";
  ooo << " :from " << from;
  ooo << " :to " << to;
  ooo << " :expr ";
  expr->lisp(ooo);
  ooo << ")";
}

/**/
void Binary::lisp(std::ostream& ooo)
{
  ooo << "#S(ast-binary";
  ooo << " :oper \"" << oper;
  ooo << "\" :type \"" << type;
  ooo << "\" :expro ";
  expro->lisp(ooo);
  ooo << " :expri ";
  expri->lisp(ooo);
  ooo << ")";
}

/**/
void FuncCall::lisp(std::ostream& ooo)
{
  ooo << "#S(ast-func-call";
  ooo << " :name " << name;
  ooo << " :args '(";
  for( auto e : args ) {
    ooo << " ";
    e->lisp(ooo);
  }
  ooo << "))";
}

/**/
void Sequence::lisp(std::ostream& ooo) 
{
  ooo << "#S(ast-sequence";
  ooo << " :sto ";
  sto->lisp(ooo);
  ooo << " :sti ";
  sti->lisp(ooo);
  ooo << ")";
}

/**/
void Declare::lisp(std::ostream& ooo) 
{
  ooo << "#S(ast-declare";
  ooo << " :name \"" << name;
  ooo << "\" :type \"" << type;
  ooo << "\")";
}

/**/
void Result::lisp(std::ostream& ooo) 
{
  ooo << "#S(ast-result :exp ";
  exp->lisp(ooo);
  ooo << ")";
}

/**/
void Assign::lisp(std::ostream& ooo) 
{
  ooo << "#S(ast-assign";
  ooo << " :name \"" << name;
  ooo << "\" :expr ";
  expr->lisp(ooo);
  ooo << ")";
}

/**/
void Branch::lisp(std::ostream& ooo)
{
  ooo << "#S(ast-branch";
  ooo << " :cond ";
  cond->lisp(ooo);
  ooo << " :thenp ";
  thenp->lisp(ooo);
  ooo << " :elsep ";
  if( elsep != nullptr )
    elsep->lisp(ooo);
  else
    ooo << "nil";
  ooo << ")";
}

/**/
void ForLoop::lisp(std::ostream& ooo)
{
  ooo << "#S(ast-for-loop";
  ooo << " :param " << param;
  ooo << " :start ";
  start->lisp(ooo);
  ooo << " :stop ";
  stop->lisp(ooo);
  ooo << " :step ";
  step->lisp(ooo);
  ooo << " :body ";
  body->lisp(ooo);
  ooo << ")";
}

/**/
void WhileLoop::lisp(std::ostream& ooo)
{
  ooo << "#S(ast-while-loop";
  ooo << " :cond ";
  cond->lisp(ooo);
  ooo << " :body ";
  body->lisp(ooo);
  ooo << ")";
}

/**/
void Input::lisp(std::ostream& ooo)
{
  ooo << "#S(ast-input";
  ooo << " :vars '(";
  for( auto& vn : vars )
    ooo << " " << vn;
  ooo << "))";
}

/**/
void Print::lisp(std::ostream& ooo)
{
  ooo << "#S(ast-print";
  ooo << " :vals '(";
  for( auto v : vals ) {
    ooo << " ";
    v->lisp(ooo);
  }
  ooo << "))";
}

