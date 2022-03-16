
#ifndef AST_HXX
#define AST_HXX

#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <utility>

//
namespace basic {

//! @brief Աբստրակտ քերականական ծառի հանգույցի տեսակը
enum class NodeKind : int {
    Empty,      //!< ոչինչ
    Boolean,    //!< տրամաբանական հաստատուն
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
class Node {
public:
    Node() = default;
    Node(NodeKind k) : kind{k} {}
    virtual ~Node() = default;

public:
    NodeKind kind = NodeKind::Empty; //!< հանգույցի տեսակը
    unsigned int line = 0;           //!< տողի համարը
};
using NodePtr = std::shared_ptr<Node>;

template<typename P, typename... Args>
std::shared_ptr<P> node(Args&&... args)
{
    return std::make_shared<P>(std::forward<Args>(args)...);
}

//! @brief Տվյալների տիպերը
//!
//! @c Void -ն օգտագործվում է արժեք չվերադարձնող ենթածրագրերի հետ աշխատելիս։
enum class Type : char {
    Void    = 'V', //!< արժեքի բացակայություն
    Boolean = 'B', //!< տրամաբանական արժեք
    Numeric = 'N', //!< թվային արժեք
    Textual = 'T'  //!< տեքստային արժեք
};

//! @brief Իդենտիֆիկատորի տիպը
//!
//! Իդենտիֆիկատորի տիպը որոշում է ըստ նրա կառուցվածքի.
//! եթե ավարտվում է '$' նիշով, ապա @c TEXTUAL է, իսկ եթե
//! ավարտվում է '?' նիշով, ապա @c BOOLEAN է, հակառակ 
//! դեպքում՝ @c NUMERIC է։
//!
Type typeOf(std::string_view name);
const std::string toString(Type vl);


//! @brief Արտահայտություն
class Expression : public Node {
public:
    Expression(NodeKind k, Type t)
        : Node{k}, type{t}
    {}

    virtual bool is(Type ty);
    virtual bool isNot(Type ty);

    Type type{Type::Void};
};
using ExpressionPtr = std::shared_ptr<Expression>;


//! @brief Տրամաբանական հաստատուն
class Boolean : public Expression {
public:
    Boolean(bool vl)
        : Expression{NodeKind::Boolean, Type::Boolean},
          value{vl}
    {}

    bool value{false};
};
using BooleanPtr = std::shared_ptr<Boolean>;


//! @brief Թվային հաստատուն
class Number : public Expression {
public:
    Number(double vl)
        : Expression{NodeKind::Number, Type::Numeric},
          value{vl}
    {}

    double value{0.0};
};
using NumberPtr = std::shared_ptr<Number>;


//! @brief Տեքստային հաստատուն
class Text : public Expression {
public:
    Text(std::string_view vl)
        : Expression{NodeKind::Text, Type::Textual},
          value{vl}
    {}

    std::string value;
};
using TextPtr = std::shared_ptr<Text>;
  

//! @brief Փոփոխական
class Variable : public Expression {
public:
    Variable(std::string_view nm)
        : Expression{NodeKind::Variable, typeOf(nm)},
          name{nm}
    {}

    std::string name; //!< փոփոխականի անունը
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
const std::string toString(Operation opc);

//! @brief Ունար գործողություն
class Unary : public Expression {
public:
    Unary(Operation op, ExpressionPtr ex)
        : Expression{NodeKind::Unary, Type::Numeric},
          opcode{op}, subexpr{ex}
    {}

    Operation opcode;       //!< գործողությոն կոդը
    ExpressionPtr subexpr;  //!< օպերանդը
};
using UnaryPtr = std::shared_ptr<Unary>;


//! @brief Բինար գործողություն
class Binary : public Expression {
public:
    Binary(Operation op, ExpressionPtr exo, ExpressionPtr exi)
        : Expression{NodeKind::Binary, Type::Void},
          opcode{op}, left{exo}, right{exi}
    {}

    Operation opcode;    //!< գործողությոն կոդը
    ExpressionPtr left;  //!< ձախ օպերանդը
    ExpressionPtr right; //!< աջ օպերանդը
};
using BinaryPtr = std::shared_ptr<Binary>;


class Subroutine;
using SubroutinePtr = std::shared_ptr<Subroutine>;
  
//! @brief Ֆունկցիայի կանչ (կիրառում)
class Apply : public Expression {
public:
    Apply(SubroutinePtr sp, const std::vector<ExpressionPtr>& ags)
        : Expression{NodeKind::Apply, Type::Void},
          callee{sp}, arguments{ags}
    {}

    SubroutinePtr callee;      //!< կանչվող ենթածրագիրը
    std::vector<ExpressionPtr> arguments; //!< արգումենտները
};
using ApplyPtr = std::shared_ptr<Apply>;


//! @brief Ղեկավարող կառուցվածք (հրաման)
class Statement : public Node {
public:
    explicit Statement(NodeKind k) : Node{k} {}
};
using StatementPtr = std::shared_ptr<Statement>;


//! @brief Հրամանների շարք (հաջորդականություն)
class Sequence : public Statement {
public:
    Sequence()
        : Statement{NodeKind::Sequence}
    {}

    std::vector<StatementPtr> items;
};
using SequencePtr = std::shared_ptr<Sequence>;


//! @brief Տվյալների ներմուծում
class Input : public Statement {
public:
    Input(TextPtr pr, VariablePtr vp)
        : Statement{NodeKind::Input},
          prompt{pr}, place{vp}
    {}

    TextPtr prompt;      //!< ներմուծման հրավերք
    VariablePtr place;   //!< ներմուծվող փոփոխական
};
using InputPtr = std::shared_ptr<Input>;
  

//! @brief Տվյալների արտածում
class Print : public Statement {
public:
    Print(ExpressionPtr ex)
        : Statement{NodeKind::Print}, expr(ex)
    {}

    ExpressionPtr expr; //!< արտածվող արտահայտությունը
};
using PrintPtr = std::shared_ptr<Print>;


//! @brief Վերագրում (միաժամանակ՝ փոփոխականի սահմանում)
class Let : public Statement {
public:
    Let(VariablePtr vp, ExpressionPtr ex)
        : Statement{NodeKind::Let},
          place{vp}, expr{ex}
    {}

    VariablePtr place;  //!< փոփոխականը
    ExpressionPtr expr; //!< արժեքը
};
using LetPtr = std::shared_ptr<Let>;


//! @brief Ճյուղավորում
class If : public Statement {
public:
    If(ExpressionPtr co, StatementPtr de, StatementPtr al = nullptr)
        : Statement{NodeKind::If}, condition{co},
          decision{de}, alternative{al}
    {}

    ExpressionPtr condition;  //!< ճյուղավորման պայման
    StatementPtr decision;    //!< @c then ճյուղը
    StatementPtr alternative; //!< @c else ճյուղը
};
using IfPtr = std::shared_ptr<If>;


//! @brief Նախապայմանով ցիկլ
class While : public Statement {
public:
    While(ExpressionPtr co, StatementPtr bo)
        : Statement{NodeKind::While}, condition{co}, body{bo}
    {}

    ExpressionPtr condition; //!< կրկնման պայման
    StatementPtr body;       //!< ցիկլի մարմինը
};
using WhilePtr = std::shared_ptr<While>;


//! @brief Պարամետրով ցիկլ
class For : public Statement {
public:
    For(VariablePtr pr, ExpressionPtr be, ExpressionPtr en,
        NumberPtr st, StatementPtr bo)
        : Statement{NodeKind::For}, parameter{pr},
          begin{be}, end{en}, step{st}, body{bo}
    {}

    VariablePtr parameter; //!< ցիկլի պարամետրը
    ExpressionPtr begin;   //!< պարամետրի սկզբնակական արժեքը
    ExpressionPtr end;     //!< պարամետրի սահմանային արժեքը
    NumberPtr step;        //!< պարամետրի փոփոխման քայլը
    StatementPtr body;     //!< ցիկլի մարմինը
};
using ForPtr = std::shared_ptr<For>;


//! @brief Ենթածրագրի կանչ
class Call : public Statement {
public:
    Call(SubroutinePtr sp, const std::vector<ExpressionPtr>& as)
        : Statement{NodeKind::Call},
          subrCall{std::make_shared<Apply>(sp, as)}
    {}

    ApplyPtr subrCall;
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
class Subroutine : public Node {
public:
    Subroutine(std::string_view nm, const std::vector<std::string>& ps)
        : Node{NodeKind::Subroutine}, 
          name{nm}, parameters{ps}
    {}

    std::string name = "";               //<! անուն
    std::vector<std::string> parameters; //<! պարամետրեր
    bool isBuiltIn = false;              //<! true - ներդրված ենթածրագրերի համար
    std::vector<VariablePtr> locals;     //<! լոկալ փոփոխականներ
    StatementPtr body;                   //<! մարմին
    bool hasValue = false;               //<! վերադարձնո՞ւմ է արժեք
};


//! @brief Ծրագիր
class Program : public Node {
public:
    Program(std::string_view fn )
        : Node{NodeKind::Program}, filename{fn}
    {}

    std::string filename;               //!< անունը
    std::vector<SubroutinePtr> members; //!< ենթածրագրերի ցուցակը
};
using ProgramPtr = std::shared_ptr<Program>;

} // basic

#endif // AST_HXX
