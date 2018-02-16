
#ifndef AST_HXX
#define AST_HXX

#include <list>
#include <sstream>
#include <string>
#include <vector>

//
namespace basic {

  // Աբստրակտ քերականական ծառի հանգույցի տեսակը
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
  
  // Քերականական ծառի հանգույցի բազային տիպը։
  // Ծառի հանգույցների բոլոր տեսակներն այս տիպի ընդլայնում են։
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
    // բոլոր դինամիկ ստեղծված հանգույցների հասցեները
    static std::list<AstNode*> allocatedNodes;
    
  public:
    static void deleteAllocatedNodes();
  };


  // Տվյալների տիպերը։ Void-ն օգտագործվում է
  // արժեք չվերադարձնող ենթածրագրերի հետ աշխատելիս։
  enum class Type : char {
    Void   = 'V',
    Number = 'N',
    Text   = 'T'  
  };


  // Արտահայտություն։
  class Expression : public AstNode {
  public:
    Type type = Type::Void;
  };

  // Թիվ
  class Number : public Expression {
  public:
    double value = 0.0;

  public:
    Number( double vl );
  };

  // Տեքստ
  class Text : public Expression {
  public:
    std::string value = "";

  public:
    Text( const std::string& vl );
  };

  // Փոփոխական
  class Variable : public Expression {
  public:
    std::string name = "";

  public:
    Variable( const std::string& nm );
  };

  // Գործողությունների անունները
  enum class Operation {
    None,
    Add, Sub, Mul, Div, Mod, Pow,
    Eq, Ne, Gt, Ge, Lt, Le,
    And, Or, Not,
    Conc
  };

  // Ունար գործողություն
  class Unary : public Expression {
  public:
    Operation opcode = Operation::None;
    Expression* subexpr = nullptr;

  public:
    Unary( Operation op, Expression* ex );
  };

  // Բինար գործողություն
  class Binary : public Expression {
  public:
    Operation opcode = Operation::None;
    Expression* subexpro = nullptr;
    Expression* subexpri = nullptr;

  public:
    Binary( Operation op, Expression* exo, Expression* exi );
  };

  class Subroutine;

  // Ֆունկցիայի կանչ (կիրառում)
  class Apply : public Expression {
  public:
    Subroutine* procptr = nullptr;
    std::vector<Expression*> arguments;

  public:
    Apply( Subroutine* sp, const std::vector<Expression*>& ags );
  };

  
  // Ղեկավարող կառուցվածք (հրաման)
  class Statement : public AstNode {};

  // Հրամանների շարք (հաջորդականություն)
  class Sequence : public Statement {
  public:
    std::vector<Statement*> items;
    
  public:
    Sequence();
  };
  
  // Տվյալների ներմուծում
  class Input : public Statement {
  public:
    Variable* varptr = nullptr;
    
  public:
    Input( Variable* vp );
  };

  // Տվյալների արտածում
  class Print : public Statement {
  public:
    Expression* expr = nullptr;
    
  public:
    Print( Expression* ex );
  };
  
  // Վերագրում (միաժամանակ՝ փոփոխականի սահմանում)
  class Let : public Statement {
  public:
    Variable* varptr = nullptr;
    Expression* expr = nullptr;
    
  public:
    Let( Variable* vp, Expression* ex );
  };
  
  // Ճյուղավորում
  class If : public Statement {
  public:
    Expression* condition = nullptr;
    Statement* decision = nullptr;
    Statement* alternative = nullptr;
    
  public:
    If( Expression* co, Statement* de, Statement* al = nullptr );
  };
  
  // Նախապայմանով ցիկլ
  class While : public Statement {
  public:
    Expression* condition = nullptr;
    Statement* body = nullptr;

  public:
    While( Expression* co, Statement* bo );
  };
  
  // Պարամետրով ցիկլ
  class For : public Statement {
  public:
    Variable* parameter = nullptr;
    Expression* begin = nullptr;
    Expression* end = nullptr;
    Expression* step = nullptr;
    Statement* body = nullptr;
    
  public:
    For( Variable* pr, Expression* be, Expression* en, Expression* st, Statement* bo );
  };

  // Ենթածրագրի կանչ
  class Call : public Statement {
  public:
    Apply* subrcall = nullptr;
    
  public:
    Call( Subroutine* sp, const std::vector<Expression*> as );
  };

  // Ենթածրագիր
  class Subroutine : public AstNode {
  public:
    std::string name = "";                // անուն
    std::vector<std::string> parameters;  // պարամետրեր
    std::vector<Variable*> locals;        // լոկալ փոփոխականներ
    Statement* body = nullptr;            // մարմին
    Type rettype = Type::Void;            // վերադարձրած արժեքի տիպ
    
  public:
    Subroutine( const std::string& nm, const std::vector<std::string>& ps, Statement* bo );
  };

  // Ծրագիր
  class Program : public AstNode {
  public:
    std::string filename = "";
    std::vector<Subroutine*> members;

  public:
    Program( const std::string& fn );
  };

} // basic

#endif // AST_HXX

