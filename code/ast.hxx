
#ifndef ABSTRACT_SYNTAX_TREE_HXX
#define ABSTRACT_SYNTAX_TREE_HXX

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "symtab.hxx"

/**/
using symbolvector = std::vector<Symbol>;

/** @brief Կոդի գեներացիայի ինտերֆեյս */
class CodeIR {
public:
  /** @brief Արտահայտությունների համար վերադարձնում է llvm::Value*, 
   իսկ հրամանների համար՝ nullptr։ */
  virtual llvm::Value* code(llvm::IRBuilder<>&) = 0;
};

/* @brief Աբստրակտ քերականական ծառը Lisp կառուցվածքների
   տեսքով արտածող ինտերֆեյս։ */
class LispAst {
public:
  /** @brief Հոսքի մեջ գրում է ԱՔԾ֊ի Lisp ներկայացումը։ */
  virtual void lisp(std::ostream&) = 0;
};

/**/
class Statement;
class Function;

/* ---------------------------------------------------------------- */
/** @brief Կոմպիլյացիայի մոդուլը */
class Module : public CodeIR, public LispAst {
public:
  /// @brief Մոդուլի անունը
  std::string name;

private:
  /// @brief LLVM մոդուլի ցուցիչը
  llvm::Module* module;
  /// @brief Մոդուլի մեջ սահմանված ենթածրագրերի ցուցակը
  std::vector<Function*> subroutines;

public:
  Module(const std::string&);
  ~Module();
  /// @brief Ավելացնել նոր ֆունկցիա
  void addFunction(Function*);
  llvm::Module* getCompiled() { return module; }

  llvm::Value* code(llvm::IRBuilder<>&) override;
  void lisp(std::ostream&) override;

private:
  void addInternal(const std::string&, const std::vector<llvm::Type*>&, llvm::Type*);
};

/* ---------------------------------------------------------------- */
/** @brief BASIC-IR լեզվի ֆունկցիա կամ պրոցեդուրա։ */
class Function : public CodeIR, public LispAst {
public:
  /// @brief Ենթածրագրի անուն։
  std::string name;
  /// @brief Արգումենտների ցուցակ։
  symbolvector args;
  /// @brief Վերադարձվող արժեքի տիպ։
  std::string type;
  /// @brief Մարմին։
  Statement* body = nullptr;
public:
  /// @brief Ցուցիչ LLVM մոդուլին։
  llvm::Module* module = nullptr;
  /// @brief Ենթածրագրում օգտագործված լոկալ անուններ։
  std::map<std::string,llvm::Value*> locals;
public:
  Function(const std::string&, const symbolvector&, const std::string&);
  ~Function();
  void setModule(llvm::Module*);
  void setBody(Statement*);
  llvm::Value* code(llvm::IRBuilder<>&) override;
  void lisp(std::ostream&) override;
};

/* ---------------------------------------------------------------- */
/** @brief Արտահայտությունների ինտերֆեյս։ */
class Expression : public CodeIR, public LispAst {
public:
  static const std::string TyBoolean;
  static const std::string TyInteger;
  static const std::string TyDouble;
  static const std::string TyVoid;
protected:
  Function* env{nullptr};
public:
  /// @brief Տիպը (ժառանգվող ատրիբուտ)։
  std::string type;
public:
  virtual ~Expression() {}
  virtual void setEnv(Function* e) { env = e; }
};

/** @brief Փոփոխականի ներկայացումը։ */
class Variable : public Expression {
private:
  /// @brief Փոփոխականի անունը։
  std::string name;
public:
  Variable(const std::string& n, const std::string& t)
    : name{n} { type = t; }
  llvm::Value* code(llvm::IRBuilder<>&) override;
  void lisp(std::ostream&) override;
};

/**/
class Constant : public Expression {
private:
  std::string value;
public:
  Constant(const std::string& vl, const std::string& ty)
    : value{vl} { type = ty; }
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
  std::string to;
public:
  TypeCast(Expression* e, const std::string& t)
    : expr{e}, to{t} { type = to; }
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
  Function* env = nullptr;

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
  void setEnv(Function*) override;
  llvm::Value* code(llvm::IRBuilder<>&) override;
  void lisp(std::ostream&) override;
};

/**/
class Input : public Statement {
private:
  symbolvector vars;
public:
  Input(const symbolvector& vs)
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

