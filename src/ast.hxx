
#ifndef AST_HXX
#define AST_HXX

#include <string>
#include <vector>

//
namespace basic {

  //
  class AstNode {
  public:
    virtual ~AstNode() = 0;
  };

  //
  class Expression : public AstNode {
  public:
    virtual ~Expression() = default;
  };
  
  //
  class Double : public Expression {
  public:
    double value = 0.0;
    
  public:
    Double( double vl );
  };

  //
  class String : public Expression {
  public:
    std::string value = "";

  public:
    String( const std::string& vl );
  };

  //
  class Variable : public Expression {
  public:
    std::string name = "";

  public:
    Variable( const std::string& nm );
  };

  //
  enum class Operation {
    None,
    Add, Sub, Mul, Div, Mod, Pow,
    Eq, Ne, Gt, Ge, Lt, Le,
    And, Or, Not
  };

  //
  class Unary : public Expression {
  public:
    Operation opcode = Operation::None;
    Expression* subexpr = nullptr;

  public:
    Unary( Operation op, Expression* ex );
    ~Unary();
  };

  //
  class Binary : public Expression {
  public:
    Operation opcode = Operation::None;
    Expression* subexpro = nullptr;
    Expression* subexpri = nullptr;

  public:
    Binary( Operation op, Expression* exo, Expression* exi );
    ~Binary();
  };

  //
  class Apply : public Expression {
  public:
    std::string procname = "";
    std::vector<Expression*> arguments;

  public:
    Apply( const std::string& pn, const std::vector<Expression*>& ags );
    ~Apply();
  };

  //
  class Statement : public AstNode {
  public:
    virtual ~Statement() = default;
  };

  //
  class Sequence : public Statement {
  public:
    std::vector<Statement*> items;
    
  public:
    Sequence() = default;
    ~Sequence();
  };
  
  //
  class Input : public Statement {
  public:
    std::string varname = "";
    
  public:
    Input( const std::string& vn );
  };

  //
  class Print : public Statement {
  public:
    Expression* expr = nullptr;
    
  public:
    Print( Expression* ex );
    ~Print();
  };
  
  //
  class Let : public Statement {
  public:
    std::string varname = "";
    Expression* expr = nullptr;
    
  public:
    Let( const std::string& vn, Expression* ex );
    ~Let();
  };
  
  //
  class If : public Statement {
  public:
    Expression* condition = nullptr;
    Statement* decision = nullptr;
    Statement* alternative = nullptr;
    
  public:
    If( Expression* co, Statement* de, Statement* al = nullptr );
    ~If();
  };
  
  //
  class While : public Statement {
  public:
    Expression* condition = nullptr;
    Statement* body = nullptr;

  public:
    While( Expression* co, Statement* bo );
    ~While();
  };
  
  //
  class For : public Statement {
  public:
    std::string parameter = "";
    Expression* begin = nullptr;
    Expression* end = nullptr;
    Expression* step = nullptr;
    Statement* body = nullptr;
    
  public:
    For( const std::string& pr, Expression* be, Expression* en, Expression* st, Statement* bo );
    ~For();
  };

  //
  class Call : public Statement {
  public:
    Apply* subrcall = nullptr;
    
  public:
    Call( const std::string& sn, const std::vector<Expression*> as );
    ~Call();
  };

  //
  class Subroutine : public AstNode {
  public:
    std::string name = "";
    std::vector<std::string> parameters;
    Statement* body = nullptr;
    
  public:
    Subroutine( const std::string& nm, const std::vector<std::string>& ps, Statement* bo = nullptr );
    ~Subroutine();
  };

  //
  class Program : public AstNode {
  public:
    std::string& filename = "";
    std::vector<Subroutine*> members;
    
  public:
    Program( const std::string& fn );
    ~Program();
  };
}

#endif // AST_HXX

