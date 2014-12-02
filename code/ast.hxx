
#ifndef ABSTRACT_SYNTAX_TREE
#define ABSTRACT_SYNTAX_TREE

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

#include <ostream>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "symtab.hxx"

/**/
using nametype = std::pair<std::string,std::string>;
using vectornametype = std::vector<nametype>;

/* Կոդի գեներացիայի ինտերֆեյս */
class CodeIR {
public:
  /* Արտահայտությունների համար վերադարձնում է llvm::Value*, 
   իսկ հրամանների համար՝ nullptr։ */
  virtual llvm::Value* code(llvm::IRBuilder<>&) = 0;
};

/* Աբստրակտ քերականական ծառը Lisp կառուցվածքների
   տեսքով արտածող ինտերֆեյս։ */
class LispAst {
public:
  virtual void lisp(std::ostream&) = 0;
};

/**/
class Statement;
class Function;

/* ---------------------------------------------------------------- */
class Module {
private:
  std::string name;
  llvm::Module* module;
  std::vector<Function*> subs;

public:
  Module(const std::string&);
  ~Module();
  void addFunction(Function*);
  void code(const std::string&);
  void lisp(const std::string&);
};

/* ---------------------------------------------------------------- */
class Function : public CodeIR, public LispAst {
public:
  std::string name;
  vectornametype args;
  std::string type;
  Statement* body = nullptr;
public:
  llvm::Module* module = nullptr;
  std::map<std::string,llvm::Value*> locals;
public:
  Function(const std::string&, const vectornametype&, const std::string&);
  ~Function();
  void setModule(llvm::Module*);
  void setBody(Statement*);
  llvm::Value* code(llvm::IRBuilder<>&) override;
  void lisp(std::ostream&) override;
};

/* ---------------------------------------------------------------- */
class Expression : public CodeIR, public LispAst {
protected:
  Function* env{nullptr};
public:
  std::string type;
public:
  virtual ~Expression() {}
  virtual void setEnv(Function* e) { env = e; }
};

/**/
class Variable : public Expression {
private:
  std::string name;
public:
  Variable(const std::string& n, const std::string& t)
    : name{n} { type = t; }
  llvm::Value* code(llvm::IRBuilder<>&) override;
  void lisp(std::ostream&) override;
};

/**/
class Boolean : public Expression {
private:
  bool value;
public:
  Boolean(bool v) : value{v} { type = "Boolean"; }
  llvm::Value* code(llvm::IRBuilder<>&) override;
  void lisp(std::ostream&) override;
};

/**/
class Integer : public Expression {
private:
  int value;
public:
  Integer(int v) : value{v} { type = "Integer"; }
  llvm::Value* code(llvm::IRBuilder<>&) override;
  void lisp(std::ostream&) override;
};


/**/
class Double : public Expression {
private:
  double value;
public:
  Double(double v) : value{v} { type = "Double"; }
  llvm::Value* code(llvm::IRBuilder<>&) override;
  void lisp(std::ostream&) override;
};

/**/
class Unary : public Expression {
private:
  std::string oper;
  Expression* expr;
public:
  Unary(const std::string&, Expression*);
  ~Unary() { delete expr; }
  void setEnv(Function*) override;
  llvm::Value* code(llvm::IRBuilder<>&) override;
  void lisp(std::ostream&) override;
};

/**/
class TypeCast : public Expression {
private:
  Expression* expr;
  std::string from;
  std::string to;
public:
  TypeCast(Expression* e, const std::string& f, const std::string& t)
    : expr{e}, from{f}, to{t} { type = to; }
  ~TypeCast() { delete expr; }
  void setEnv(Function*) override;
  llvm::Value* code(llvm::IRBuilder<>&) override;
  void lisp(std::ostream&) override;
};

/**/
class Binary : public Expression {
private:
  static std::set<std::string> Numerics;
  static std::set<std::string> Logicals;
private:
  std::string oper;
  Expression* expro;
  Expression* expri;
public:
  Binary(const std::string&, Expression*, Expression*);
  ~Binary();
  void setEnv(Function*) override;
  llvm::Value* code(llvm::IRBuilder<>&) override;
  void lisp(std::ostream&) override;
};

/**/
class FuncCall : public Expression {
private:
  std::string name;
  std::vector<Expression*> args;
public:
  FuncCall(const std::string& nm, const std::vector<Expression*>& ag)
    : name{nm}, args{ag}
  {}
  ~FuncCall();
  void setEnv(Function*) override;
  llvm::Value* code(llvm::IRBuilder<>&) override;
  void lisp(std::ostream&) override;
};

/* ---------------------------------------------------------------- */
class Statement : public CodeIR, public LispAst {
protected:
  Function* env;
public:
  virtual ~Statement() {}
  virtual void setEnv(Function* e) { env = e; }
};

/**/
class Sequence : public Statement {
private:
  Statement* sto;
  Statement* sti;
public:
  Sequence(Statement* so, Statement* si) 
    : sto{so}, sti{si} {}
  ~Sequence();
  void setEnv(Function*) override; 
  llvm::Value* code(llvm::IRBuilder<>&) override;
  void lisp(std::ostream&) override;
};

/**/
class Declare : public Statement {
private:
  std::string name;
  std::string type;
public:
  Declare(const std::string& n, const std::string& t)
    : name{n}, type{t} {}
  llvm::Value* code(llvm::IRBuilder<>&) override;
  void lisp(std::ostream&) override;
};

/**/
class SubCall : public Statement {
private:
  FuncCall* subr;
public:
  SubCall(const std::string& nm, const std::vector<Expression*>& ag)
    : subr{new FuncCall{nm, ag}}
  {}
  ~SubCall();
  void setEnv(Function*) override;
  llvm::Value* code(llvm::IRBuilder<>&) override;
  void lisp(std::ostream&) override;
};

/**/
class Result : public Statement {
private:
  Expression* exp;
public:
  Result(Expression* e) : exp{e} {}
  ~Result();
  void setEnv(Function*) override;
  llvm::Value* code(llvm::IRBuilder<>&) override;
  void lisp(std::ostream&) override;
};

/**/
class Assign : public Statement {
private:
  std::string name;
  Expression* expr;
public:
  Assign(const std::string& n, Expression* e)
    : name{n}, expr{e} {}
  ~Assign();
  void setEnv(Function*) override;
  llvm::Value* code(llvm::IRBuilder<>&) override;
  void lisp(std::ostream&) override;
};

/**/
class Branch : public Statement {
private:
  Expression* cond;
  Statement* thenp;
  Statement* elsep;
public:
  Branch(Expression* c, Statement* t, Statement* e)
    : cond{c}, thenp{t}, elsep{e} {}
  ~Branch();
  void setElse(Statement* s) { elsep = s; }
  void setEnv(Function*) override;
  llvm::Value* code(llvm::IRBuilder<>&) override;
  void lisp(std::ostream&) override;
};

/**/
class ForLoop : public Statement {
private:
  std::string param;
  Expression* start;
  Expression* stop;
  Expression* step;
  Statement* body;
public:
  ForLoop(const std::string& pr, Expression* sa, Expression* so, Expression* se, Statement* bo)
    : param{pr}, start{sa}, stop{so}, step{se}, body{bo} {}
  ~ForLoop();
  void setEnv(Function*) override;
  llvm::Value* code(llvm::IRBuilder<>&) override;
  void lisp(std::ostream&) override;
};

/**/
class WhileLoop : public Statement {
private:
  Expression* cond;
  Statement* body;
public:
  WhileLoop(Expression* co, Statement* bo)
    : cond{co}, body{bo} {}
  ~WhileLoop();
  void setEnv(Function*);
  llvm::Value* code(llvm::IRBuilder<>&) override;
  void lisp(std::ostream&) override;
};

/**/
class Input : public Statement {
private:
  std::vector<std::string> vars;
public:
  Input(const std::vector<std::string>& vs)
    : vars{vs} {}
  llvm::Value* code(llvm::IRBuilder<>&) override;
  void lisp(std::ostream&) override;
};

/**/
class Print : public Statement {
private:
  std::vector<Expression*> vals;
public:
  Print(const std::vector<Expression*>& vl)
    : vals{vl} {}
  ~Print();
  void setEnv(Function*) override;
  llvm::Value* code(llvm::IRBuilder<>&) override;
  void lisp(std::ostream&) override;
};

#endif

