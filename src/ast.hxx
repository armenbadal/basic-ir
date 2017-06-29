
#ifndef AST_HXX
#define AST_HXX

#include <list>
#include <sstream>
#include <string>
#include <vector>

//
namespace basic {

  //
  class AstNode {
  private:
    unsigned int line = 0;

  public:
    AstNode();
    virtual ~AstNode() = default;

    virtual void lisp( std::ostringstream& ooo ) {}

  private:
    static std::list<AstNode*> allocated_nodes;
    
  public:
    static void delete_allocated_nodes();
  };


  //
  enum class Type : char {
    Void   = 'V',
    Number = 'N',
    Text   = 'T'  
  };


  //
  class Expression : public AstNode {
  public:
    Type type = Type::Void;
  };

  //
  class Number : public Expression {
  public:
    double value = 0.0;

  public:
    Number( double vl );

    void lisp( std::ostringstream& ooo );
  };

  //
  class Text : public Expression {
  public:
    std::string value = "";

  public:
    Text( const std::string& vl );

    void lisp( std::ostringstream& ooo );
  };

  //
  class Variable : public Expression {
  public:
    std::string name = "";

  public:
    Variable( const std::string& nm );

    void lisp( std::ostringstream& ooo );
  };

  //
  enum class Operation {
    None,
    Add, Sub, Mul, Div, Mod, Pow,
    Eq, Ne, Gt, Ge, Lt, Le,
    And, Or, Not,
    Conc
  };

  //
  class Unary : public Expression {
  public:
    Operation opcode = Operation::None;
    Expression* subexpr = nullptr;

  public:
    Unary( Operation op, Expression* ex );

    void lisp( std::ostringstream& ooo );
  };

  //
  class Binary : public Expression {
  public:
    Operation opcode = Operation::None;
    Expression* subexpro = nullptr;
    Expression* subexpri = nullptr;

  public:
    Binary( Operation op, Expression* exo, Expression* exi );

    void lisp( std::ostringstream& ooo );
  };

  //
  class Apply : public Expression {
  public:
    std::string procname = "";
    std::vector<Expression*> arguments;

  public:
    Apply( const std::string& pn, const std::vector<Expression*>& ags );

    void lisp( std::ostringstream& ooo );
  };

  
  //
  class Statement : public AstNode {};

  //
  class Sequence : public Statement {
  public:
    std::vector<Statement*> items;
    
  public:
    Sequence() = default;

    void lisp( std::ostringstream& ooo );
  };
  
  //
  class Input : public Statement {
  public:
    std::string varname = "";
    
  public:
    Input( const std::string& vn );

    void lisp( std::ostringstream& ooo );
  };

  //
  class Print : public Statement {
  public:
    Expression* expr = nullptr;
    
  public:
    Print( Expression* ex );

    void lisp( std::ostringstream& ooo );
  };
  
  //
  class Let : public Statement {
  public:
    std::string varname = "";
    Expression* expr = nullptr;
    
  public:
    Let( const std::string& vn, Expression* ex );

    void lisp( std::ostringstream& ooo );
  };
  
  //
  class If : public Statement {
  public:
    Expression* condition = nullptr;
    Statement* decision = nullptr;
    Statement* alternative = nullptr;
    
  public:
    If( Expression* co, Statement* de, Statement* al = nullptr );

    void lisp( std::ostringstream& ooo );
  };
  
  //
  class While : public Statement {
  public:
    Expression* condition = nullptr;
    Statement* body = nullptr;

  public:
    While( Expression* co, Statement* bo );

    void lisp( std::ostringstream& ooo );
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

    void lisp( std::ostringstream& ooo );
  };

  //
  class Call : public Statement {
  public:
    Apply* subrcall = nullptr;
    
  public:
    Call( const std::string& sn, const std::vector<Expression*> as );

    void lisp( std::ostringstream& ooo );
  };

  //
  class Subroutine : public AstNode {
  public:
    std::string name = "";
    std::vector<std::string> parameters;
    Statement* body = nullptr;
    
  public:
    Subroutine( const std::string& nm, const std::vector<std::string>& ps, Statement* bo );

    void lisp( std::ostringstream& ooo );
  };

  //
  class Program : public AstNode {
  public:
    std::string filename = "";
    std::vector<Subroutine*> members;

  public:
    Program( const std::string& fn );

    void lisp( std::ostringstream& ooo );
  };

} // basic

#endif // AST_HXX

