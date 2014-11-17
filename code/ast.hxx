
#ifndef ABSTRACT_SYNTAX_TREE
#define ABSTRACT_SYNTAX_TREE

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

#include <string>
#include <utility>
#include <vector>

/**/
using pairofstrings = std::pair<std::string,std::string>;
using vectorofpairsofstrings = std::vector<pairofstrings>;

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
  void addFunction(Function*);
  void code(const std::string&);
};

/* ---------------------------------------------------------------- */
class Function {
private:
  std::string name;
  vectorofpairsofstrings args;
  std::string type;
  Statement* body;
public:
  llvm::Module* module;
  std::map<std::string,llvm::Value*> locals;
public:
  Function(const std::string&, const vectorofpairsofstrings&, const std::string&);
  void setModule(llvm::Module*);
  void setBody(Statement*);
  llvm::Function* code();
};

/* ---------------------------------------------------------------- */
class Expression {
protected:
  Function* env;
public:
  virtual ~Expression() {}
  virtual void setEnv(Function* e) { env = e; }
  virtual llvm::Value* code(llvm::IRBuilder<>&) = 0;
};

/**/
class Variable : public Expression {
private:
  std::string name;
public:
  Variable(const std::string& n) : name{n} {}
  llvm::Value* code(llvm::IRBuilder<>&);
};

/**/
class Boolean : public Expression {
private:
  bool value;
public:
  Boolean(bool v) : value{v} {}
  llvm::Value* code(llvm::IRBuilder<>&);
};

/**/
class Integer : public Expression {
private:
  int value;
public:
  Integer(int v) : value{v} {}
  llvm::Value* code(llvm::IRBuilder<>&);
};


/**/
class Double : public Expression {
private:
  double value;
public:
  Double(double v) : value{v} {}
  llvm::Value* code(llvm::IRBuilder<>&);
};

/**/
class Unary : public Expression {
private:
  std::string oper;
  Expression* expr;
public:
  Unary(const std::string& op, Expression* ex)
    : oper{op}, expr{ex} {}
  void setEnv(Function*);
  llvm::Value* code(llvm::IRBuilder<>&);
};

/**/
class TypeCast : public Expression {
private:
  Expression* expr;
  std::string from;
  std::string to;
public:
  TypeCast(Expression* e, const std::string& f, const std::string& t)
    : expr{e}, from{f}, to{t} {}
  void setEnv(Function*);
  llvm::Value* code(llvm::IRBuilder<>&);
};

/**/
class Binary : public Expression {
private:
  std::string oper;
  Expression* expro;
  Expression* expri;
public:
  Binary(const std::string& op, Expression* exo, Expression* exi)
    : oper{op}, expro{exo}, expri{exi} {}
  void setEnv(Function*);
  llvm::Value* code(llvm::IRBuilder<>&);
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
  void setEnv(Function*);
  llvm::Value* code(llvm::IRBuilder<>&);
};

/* ---------------------------------------------------------------- */
class Statement {
protected:
  Function* env;
public:
  virtual ~Statement() {}
  virtual void setEnv(Function* e) { env = e; }
  virtual void code(llvm::IRBuilder<>&) = 0;
};

/**/
class Sequence : public Statement {
private:
  Statement* sto;
  Statement* sti;
public:
  Sequence(Statement* so, Statement* si) 
    : sto{so}, sti{si} {}
  void setEnv(Function*); 
  void code(llvm::IRBuilder<>&);
};

/**/
class Declare : public Statement {
private:
  std::string name;
  std::string type;
public:
  Declare(const std::string& n, const std::string& t)
    : name{n}, type{t} {}
  void code(llvm::IRBuilder<>&);
};

/**/
class Result : public Statement {
private:
  Expression* exp;
public:
  Result(Expression* e) : exp{e} {}
  void setEnv(Function*);
    void code(llvm::IRBuilder<>&);
};

/**/
class Assign : public Statement {
private:
  std::string name;
  Expression* expr;
public:
  Assign(const std::string& n, Expression* e)
    : name{n}, expr{e} {}
  void setEnv(Function*);
  void code(llvm::IRBuilder<>&);
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
  void setElse(Statement* s) { elsep = s; }
  void setEnv(Function*);
  void code(llvm::IRBuilder<>&);
};

/**/
class WhileLoop : public Statement {
private:
  Expression* cond;
  Statement* body;
public:
  WhileLoop(Expression* co, Statement* bo)
    : cond{co}, body{bo} {}
  void setEnv(Function*);
  void code(llvm::IRBuilder<>&);
};

/**/
class Input : public Statement {
private:
  std::vector<std::string> vars;
public:
  Input(const std::vector<std::string>& vs)
    : vars{vs} {}
  void code(llvm::IRBuilder<>&);
};

/**/
class Print : public Statement {
private:
  std::vector<Expression*> vals;
public:
  Print(const std::vector<Expression*>& vl)
    : vals{vl} {}
  void setEnv(Function*);
  void code(llvm::IRBuilder<>&);
};

#endif

