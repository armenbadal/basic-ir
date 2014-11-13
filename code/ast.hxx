
#ifndef ABSTRACT_SYNTAX_TREE
#define ABSTRACT_SYNTAX_TREE

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

#include <map>
#include <string>
#include <vector>


/**/
class Statement;

/* ---------------------------------------------------------------- */
class Function {
private:
  std::string name;
  std::map<std::string,std::string> args;
  std::string type;
  Statement* body;
  llvm::Module* module;
public:
  std::map<std::string,llvm::Value*> locals;
public:
  Function(const std::string&, const std::map<std::string,std::string>&, 
	   const std::string&, Statement*, llvm::Module*);
  void setModule(llvm::Module*);
  void Generate();
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

#endif

