
#ifndef AST_HXX
#define AST_HXX

#include <list>
#include <string>
#include <vector>
#include <memory>

//
namespace basic {

//! @brief Աբստրակտ քերականական ծառի հանգույցի տեսակը
enum class NodeKind : int {
    Empty,      //!< ոչինչ
    Number,     //!< թվային հաստատուն
    Text,       //!< տեքստային հաստատուն
    Variable,   //!< փոփոխականի հղում
    Unary,      //!< ունար գործողություն
    Binary,     //!< բինար գործողություն
    Apply,      //!< ֆունկցիայի կիրառում
    Sequence,   //!< հրամանների հաջորդում
    Input,      //!< տվյալների ներմուծում
    Print,      //!< տվյալների արտածում
    Let,        //!< վերագրում
    If,         //!< ճյուղավորում
    While,      //!< պայմանով կրկնություն
    For,        //!< պարամետրով կրկնություն
    Call,       //!< պրոցեդուրայի կանչ
    Subroutine, //!< ենթածրագիր
    Program     //!< ծրագիր
};


//! @brief Քերականական ծառի հանգույցի բազային տիպը։
//!
//! Ծառի հանգույցների բոլոր տեսակներն այս տիպի ընդլայնում են։
class AstNode {
public:
    virtual ~AstNode() = default;

public:
    NodeKind kind = NodeKind::Empty; //!< հանգույցի տեսակը
    unsigned int line = 0;           //!< տողի համարը
};
using AstNodePtr = std::shared_ptr<AstNode>;


//! @brief Տվյալների տիպերը
//!
//! @c Void -ն օգտագործվում է արժեք չվերադարձնող ենթածրագրերի հետ աշխատելիս։
enum class Type : char {
    Void =   'V', //!< արժեքի բացակայություն
    Number = 'N', //!< թվային արժեք
    Text =   'T'  //!< տեքստային արժեք
};

//! @brief Իդենտիֆիկատորի տիպը
//!
//! Իդենտիֆիկատորի տիպը որոշում է ըստ նրա կառուցվածքի.
//! եթե ավարտվում է '$' նիշով, ապա @c TEXT է, հակառակ
//! դեպքում՝ @c REAL է։
//!
Type typeOf( const std::string& nm );
std::string toString( Type vl );


//! @brief Արտահայտություն
class Expression : public AstNode {
public:
    Type type = Type::Void;
};
using ExpressionPtr = std::shared_ptr<Expression>;


//! @brief Թվային հաստատուն
class Number : public Expression {
public:
    double value = 0.0;

public:
    Number( double vl )
        : value(vl)
    {
        kind = NodeKind::Number;
        type = Type::Number;
    }
};
using NumberPtr = std::shared_ptr<Number>;


//! @brief Տեքստային հաստատուն
class Text : public Expression {
public:
    std::string value = "";

public:
    Text( const std::string& vl )
        : value(vl)
    {
        kind = NodeKind::Text;
        type = Type::Text;
    }
};
using TextPtr = std::shared_ptr<Text>;
  

//! @brief Փոփոխական
class Variable : public Expression {
public:
    std::string name = ""; //!< փոփոխականի անունը

public:
    Variable( const std::string& nm )
        : name(nm)
    {
        kind = NodeKind::Variable;
        type = typeOf(name);
    }
};
using VariablePtr = std::shared_ptr<Variable>;


//! @brief Գործողությունների անունները
enum class Operation {
    None, //!< անորոշ
    Add,  //!< գումարում
    Sub,  //!< հանում
    Mul,  //!< բազմապատկում
    Div,  //!< բաժանում
    Mod,  //!< ամբողջ բաժանում
    Pow,  //!< աստիճան
    Eq,   //!< հավասար է
    Ne,   //!< հավասար չէ
    Gt,   //!< մեծ է
    Ge,   //!< մեծ է կամ հավասար
    Lt,   //!< փոքր է
    Le,   //!< փոքր է կամ հավասար
    And,  //!< ԵՎ (կոնյունկցիա)
    Or,   //!< ԿԱՄ (դիզյունկցիա)
    Not,  //!< ՈՉ (ժխտում)
    Conc  //!< տեքստերի կցում
};

//! @brief Գործողության տեքստային անունը
std::string toString(Operation opc);

//! @brief Ունար գործողություն
class Unary : public Expression {
public:
    Operation opcode = Operation::None; //!< գործողությոն կոդը
    ExpressionPtr subexpr;      //!< օպերանդը

public:
    Unary( Operation op, ExpressionPtr ex )
        : opcode(op), subexpr(ex)
    {
        kind = NodeKind::Unary;
        type = Type::Number;
    }
};
using UnaryPtr = std::shared_ptr<Unary>;


//! @brief Բինար գործողություն
class Binary : public Expression {
public:
    Operation opcode = Operation::None; //!< գործողությոն կոդը
    ExpressionPtr subexpro;     //!< ձախ օպերանդը
    ExpressionPtr subexpri;     //!< աջ օպերանդը

public:
    Binary( Operation op, ExpressionPtr exo, ExpressionPtr exi )
        : opcode(op), subexpro(exo), subexpri(exi)
    {
        kind = NodeKind::Binary;
    }
};
using BinaryPtr = std::shared_ptr<Binary>;


class Subroutine;
using SubroutinePtr = std::shared_ptr<Subroutine>;
  
//! @brief Ֆունկցիայի կանչ (կիրառում)
class Apply : public Expression {
public:
    SubroutinePtr procptr;      //!< կանչվող ենթածրագիրը
    std::vector<ExpressionPtr> arguments; //!< արգումենտները

public:
    Apply( SubroutinePtr sp, const std::vector<ExpressionPtr>& ags )
        : procptr(sp), arguments(ags)
    {
        kind = NodeKind::Apply;
    }
};
using ApplyPtr = std::shared_ptr<Apply>;


//! @brief Ղեկավարող կառուցվածք (հրաման)
class Statement : public AstNode {};
using StatementPtr = std::shared_ptr<Statement>;


//! @brief Հրամանների շարք (հաջորդականություն)
class Sequence : public Statement {
public:
    std::vector<StatementPtr> items;

public:
    Sequence()
    {
        kind = NodeKind::Sequence;
    }
};
using SequencePtr = std::shared_ptr<Sequence>;


//! @brief Տվյալների ներմուծում
class Input : public Statement {
public:
    std::string prompt = "";    //!< ներմուծման հրավերք
    VariablePtr varptr; //!< ներմուծվող փոփոխական

public:
    Input( const std::string& pr, VariablePtr vp )
        : prompt(pr), varptr(vp)
    {
        kind = NodeKind::Input;
    }
};
using InputPtr = std::shared_ptr<Input>;
  

//! @brief Տվյալների արտածում
class Print : public Statement {
public:
    ExpressionPtr expr; //!< արտածվող արտահայտությունը

public:
    Print( ExpressionPtr ex )
        : expr(ex)
    {
        kind = NodeKind::Print;
    }
};
using PrintPtr = std::shared_ptr<Print>;


//! @brief Վերագրում (միաժամանակ՝ փոփոխականի սահմանում)
class Let : public Statement {
public:
    VariablePtr varptr; //!< փոփոխականը
    ExpressionPtr expr; //!< արժեքը

public:
    Let( VariablePtr vp, ExpressionPtr ex )
        : varptr(vp), expr(ex)
    {
        kind = NodeKind::Let;
    }
};
using LetPtr = std::shared_ptr<Let>;


//! @brief Ճյուղավորում
class If : public Statement {
public:
    ExpressionPtr condition;  //!< ճյուղավորման պայման
    StatementPtr decision;    //!< @c then ճյուղը
    StatementPtr alternative; //!< @c else ճյուղը

public:
    If( ExpressionPtr co, StatementPtr de, StatementPtr al = nullptr )
        : condition(co), decision(de), alternative(al)
    {
        kind = NodeKind::If;
    }
};
using IfPtr = std::shared_ptr<If>;


//! @brief Նախապայմանով ցիկլ
class While : public Statement {
public:
    ExpressionPtr condition; //!< կրկնման պայման
    StatementPtr body;       //!< ցիկլի մարմինը

public:
    While( ExpressionPtr co, StatementPtr bo )
        : condition(co), body(bo)
    {
        kind = NodeKind::While;
    }
};
using WhilePtr = std::shared_ptr<While>;


//! @brief Պարամետրով ցիկլ
class For : public Statement {
public:
    VariablePtr parameter; //!< ցիկլի պարամետրը
    ExpressionPtr begin;   //!< պարամետրի սկզբնակական արժեքը
    ExpressionPtr end;     //!< պարամետրի սահմանային արժեքը
    NumberPtr step;    //!< պարամետրի փոփոխման քայլը
    StatementPtr body;     //!< ցիկլի մարմինը

public:
    For( VariablePtr pr, ExpressionPtr be, ExpressionPtr en,
         NumberPtr st, StatementPtr bo )
        : parameter(pr), begin(be), end(en), step(st), body(bo)
    {
        kind = NodeKind::For;
    }
};
using ForPtr = std::shared_ptr<For>;


//! @brief Ենթածրագրի կանչ
class Call : public Statement {
public:
    ApplyPtr subrcall;

public:
    Call( SubroutinePtr sp, const std::vector<ExpressionPtr>& as )
        : subrcall(std::make_shared<Apply>(sp, as))
    {
        kind = NodeKind::Call;
    }
};
using CallPtr = std::shared_ptr<Call>;


//! @brief Ենթածրագիր
//!
//! Ենթածրագիրը օգտագործվում է և՛ որպես պրոցեդուրա, և՛ որպես
//! ֆունկցիա։ Դրա վերադարձրած արժեքի տիպը որոշվում է անվան
//! կառուցվածքով, ինչպես փոփոխականներինը։ Ենթածրագիրը 
//! ֆունկցիա է, եթե նրա մարմնում է անվանը արժեք վերագրող
//! @c LET հրաման։ Այդ դեպքում @c hasValue անդամի արժեքը
//! դրվում է @c true ։
class Subroutine : public AstNode {
public:
    std::string name = "";               //<! անուն
    std::vector<std::string> parameters; //<! պարամետրեր
    bool isBuiltIn = false;              //<! true - ներդրված ենթածրագրերի համար
    std::vector<VariablePtr> locals;     //<! լոկալ փոփոխականներ
    StatementPtr body;                   //<! մարմին
    bool hasValue = false;               //<! վերադարձնո՞ւմ է արժեք

public:
    Subroutine( const std::string& nm, const std::vector<std::string>& ps )
        : name(nm), parameters(ps)
    {
        kind = NodeKind::Subroutine;
    }
};


//! @brief Ծրագիր
class Program : public AstNode {
public:
    std::string filename = "";        //!< անունը
    std::vector<SubroutinePtr> members; //!< ենթածրագրերի ցուցակը

public:
    Program( const std::string& fn )
        : filename(fn)
    {
        kind = NodeKind::Program;
    }
};
using ProgramPtr = std::shared_ptr<Program>;

} // basic

#endif // AST_HXX
