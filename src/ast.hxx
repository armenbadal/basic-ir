
#ifndef AST_HXX
#define AST_HXX

#include <list>
#include <sstream>
#include <string>
#include <vector>

//
namespace basic {

  //
  enum class NodeKind : int {
    Empty,
    Number,
    Text,
    Variable,
    Unary,
    Binary,
    Apply,
    Sequence,
    Input,
    Print,
    Let,
    If,
    While,
    For,
    Call,
    Subroutine,
    Program
  };
  
  //
  class AstNode {
  public:
    NodeKind kind = NodeKind::Empty;

  private:    
    unsigned int line = 0;

  public:
    AstNode();
    virtual ~AstNode() = default;

	void printKind();
		
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
  };

  //
  class Text : public Expression {
  public:
    std::string value = "";

  public:
    Text( const std::string& vl );
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
  };

  //
  class Binary : public Expression {
  public:
    Operation opcode = Operation::None;
    Expression* subexpro = nullptr;
    Expression* subexpri = nullptr;

  public:
    Binary( Operation op, Expression* exo, Expression* exi );
  };

  //
  class Apply : public Expression {
  public:
    std::string procname = "";
    std::vector<Expression*> arguments;

  public:
    Apply( const std::string& pn, const std::vector<Expression*>& ags );
  };

  
  //
  class Statement : public AstNode {};

  //
  class Sequence : public Statement {
  public:
    std::vector<Statement*> items;
    
  public:
    Sequence();
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
  };
  
  //
  class Let : public Statement {
  public:
    std::string varname = ""; // TODO: սա դարձնել Variable*
    Expression* expr = nullptr;
    
  public:
    Let( const std::string& vn, Expression* ex );
  };
  
  //
  class If : public Statement {
  public:
    Expression* condition = nullptr;
    Statement* decision = nullptr;
    Statement* alternative = nullptr;
    
  public:
    If( Expression* co, Statement* de, Statement* al = nullptr );
  };
  
  //
  class While : public Statement {
  public:
    Expression* condition = nullptr;
    Statement* body = nullptr;

  public:
    While( Expression* co, Statement* bo );
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
  };

  //
  class Call : public Statement {
  public:
    Apply* subrcall = nullptr;
    
  public:
    Call( const std::string& sn, const std::vector<Expression*> as );
  };

  //
  class Subroutine : public AstNode {
  public:
    std::string name = "";
    std::vector<std::string> parameters;
	std::vector<Variable*> locals;
    Statement* body = nullptr;
    Type rettype = Type::Void;
    
  public:
    Subroutine( const std::string& nm, const std::vector<std::string>& ps, Statement* bo );
  };

  //
  class Program : public AstNode {
  public:
    std::string filename = "";
    std::vector<Subroutine*> members;

  public:
    Program( const std::string& fn );
  };

} // basic

#endif // AST_HXX

