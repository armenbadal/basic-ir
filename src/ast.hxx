#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

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
    Array,      //!< զանգվածի տարրեր
    Sequence,   //!< հրամանների հաջորդում
    Dim,        //!< զանգվածի սահմանում
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

// @brief Թոքենի դիրքը տեքստում (հիմա՝ տողը)
using Position = unsigned int;

//! @brief Քերականական ծառի հանգույցի բազային տիպը։
//!
//! Ծառի հանգույցների բոլոր տեսակներն այս տիպի ընդլայնում են։
class Node {
public:
    Node() = default;
    explicit Node(NodeKind k, Position l) : kind{k}, line{l} {}
    virtual ~Node() = default;

    using Ptr = std::shared_ptr<Node>;

    const NodeKind kind{NodeKind::Empty}; //!< հանգույցի տեսակը
    const Position line{0};               //!< տողի համարը
};

template<typename P, typename... Args>
std::shared_ptr<P> node(Args&&... args)
{
    return std::make_shared<P>(std::forward<Args>(args)...);
}


//! @brief Արտահայտություն
class Expression : public Node {
public:
    Expression(NodeKind k, Position p)
        : Node{k, p}
    {}

    using Ptr = std::shared_ptr<Expression>;
};


//! @brief Տրամաբանական հաստատուն
class Boolean : public Expression {
public:
    explicit Boolean(bool vl, Position p)
        : Expression{NodeKind::Boolean, p},
          value{vl}
    {}
    using Ptr = std::shared_ptr<Boolean>;

    const bool value{false};
};


//! @brief Թվային հաստատուն
class Number : public Expression {
public:
    explicit Number(double vl, Position p)
        : Expression{NodeKind::Number, p},
          value{vl}
    {}
    using Ptr = std::shared_ptr<Number>;

    const double value{0.0};
};


//! @brief Տեքստային հաստատուն
class Text : public Expression {
public:
    explicit Text(std::string_view vl, Position p)
        : Expression{NodeKind::Text, p},
          value{vl}
    {}
    using Ptr = std::shared_ptr<Text>;

    const std::string value;
};
  

//! @brief Փոփոխական
class Variable : public Expression {
public:
    explicit Variable(std::string_view nm, Position p)
        : Expression{NodeKind::Variable, p},
          name{nm}
    {}
    using Ptr = std::shared_ptr<Variable>;

    const std::string name; //!< փոփոխականի անունը
};


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

//! @brief Ունար գործողություն
class Unary : public Expression {
public:
    Unary(Operation op, Expression::Ptr ex, Position p)
        : Expression{NodeKind::Unary, p},
          opcode{op}, operand{ex}
    {}
    using Ptr = std::shared_ptr<Unary>;

    const Operation opcode;       //!< գործողության կոդը
    const Expression::Ptr operand;  //!< օպերանդը
};


//! @brief Բինար գործողություն
class Binary : public Expression {
public:
    Binary(Operation op, Expression::Ptr exo, Expression::Ptr exi, Position p)
        : Expression{NodeKind::Binary, p},
          opcode{op}, left{exo}, right{exi}
    {}
    using Ptr = std::shared_ptr<Binary>;

    const Operation opcode;    //!< գործողության կոդը
    const Expression::Ptr left;  //!< ձախ օպերանդը
    const Expression::Ptr right; //!< աջ օպերանդը
};

//! @brief Ֆունկցիայի կանչ (կիրառում)
class Apply : public Expression {
public:
    Apply(std::string_view sp, std::vector<Expression::Ptr> ags, Position p)
        : Expression{NodeKind::Apply, p},
          callee{sp}, arguments{std::move(ags)}
    {}
    using Ptr = std::shared_ptr<Apply>;

    const std::string callee;                     //!< կանչվող ենթածրագիրը
    const std::vector<Expression::Ptr> arguments; //!< արգումենտները
};

//! @brief Զանգվածի թվարկված տարրեր
class Array : public Expression {
public:
    Array(std::vector<Expression::Ptr> elems, Position p)
        : Expression{NodeKind::Array, p},
        elements{std::move(elems)}
    {}
    using Ptr = std::shared_ptr<Array>;

    const std::vector<Expression::Ptr> elements; //!< տարրերը
};


//! @brief Ղեկավարող կառուցվածք (հրաման)
class Statement : public Node {
public:
    explicit Statement(NodeKind k, Position p)
        : Node{k, p}
    {}
    using Ptr = std::shared_ptr<Statement>;
};


//! @brief Հրամանների շարք (հաջորդականություն)
class Sequence : public Statement {
public:
    Sequence(std::vector<Statement::Ptr> s, Position p) 
        : Statement{NodeKind::Sequence, p},
          items{std::move(s)}
    {}
    using Ptr = std::shared_ptr<Sequence>;

    const std::vector<Statement::Ptr> items;
};

//! @brief Զանգվածի սահմանում
class Dim : public Statement {
public:
    Dim(std::string_view nm, Expression::Ptr sz, Position p)
        : Statement{NodeKind::Dim, p},
          name{nm}, size{sz}
    {}
    using Ptr = std::shared_ptr<Dim>;

    const std::string name;     //!< անունը
    const Expression::Ptr size; //!< չափը
};

//! @brief Տվյալների ներմուծում
class Input : public Statement {
public:
    Input(Variable::Ptr vp, Position p)
        : Statement{NodeKind::Input, p},
          place{vp}
    {}
    using Ptr = std::shared_ptr<Input>;

    const Variable::Ptr place;   //!< ներմուծվող փոփոխական
};


//! @brief Տվյալների արտածում
class Print : public Statement {
public:
    explicit Print(Expression::Ptr ex, Position p)
        : Statement{NodeKind::Print, p}, 
          expr{ex}
    {}
    using Ptr = std::shared_ptr<Print>;

    const Expression::Ptr expr; //!< արտածվող արտահայտությունը
};


//! @brief Վերագրում (միաժամանակ՝ փոփոխականի սահմանում)
class Let : public Statement {
public:
    Let(Variable::Ptr vp, Expression::Ptr ex, Position p)
        : Statement{NodeKind::Let, p},
          place{vp}, expr{ex}
    {}
    using Ptr = std::shared_ptr<Let>;

    const Variable::Ptr place;  //!< փոփոխականը
    const Expression::Ptr expr; //!< արժեքը
};


//! @brief Ճյուղավորում
class If : public Statement {
public:
    If(Expression::Ptr co, Statement::Ptr de, Statement::Ptr al, Position p)
        : Statement{NodeKind::If, p}, 
          condition{co}, decision{de}, alternative{al}
    {}
    using Ptr = std::shared_ptr<If>;

    const Expression::Ptr condition;  //!< ճյուղավորման պայման
    const Statement::Ptr decision;    //!< @c then ճյուղը
    const Statement::Ptr alternative; //!< @c else ճյուղը
};


//! @brief Նախապայմանով ցիկլ
class While : public Statement {
public:
    While(Expression::Ptr co, Statement::Ptr bo, Position p)
        : Statement{NodeKind::While, p}, 
          condition{co}, body{bo}
    {}
    using Ptr = std::shared_ptr<While>;

    const Expression::Ptr condition; //!< կրկնման պայման
    const Statement::Ptr body;       //!< ցիկլի մարմինը
};


//! @brief Պարամետրով ցիկլ
class For : public Statement {
public:
    For(Variable::Ptr pr, Expression::Ptr be, Expression::Ptr en,
        Number::Ptr st, Statement::Ptr bo, Position p)
        : Statement{NodeKind::For, p},
          parameter{pr}, begin{be}, end{en}, step{st}, body{bo}
    {}
    using Ptr = std::shared_ptr<For>;

    const Variable::Ptr parameter; //!< ցիկլի պարամետրը
    const Expression::Ptr begin;   //!< պարամետրի սկզբնակական արժեքը
    const Expression::Ptr end;     //!< պարամետրի սահմանային արժեքը
    const Number::Ptr step;        //!< պարամետրի փոփոխման քայլը
    const Statement::Ptr body;     //!< ցիկլի մարմինը
};


//! @brief Ենթածրագրի կանչ
class Call : public Statement {
public:
    Call(std::string_view sp, std::vector<Expression::Ptr> as, Position p)
        : Statement{NodeKind::Call, p},
          subrCall{node<Apply>(sp, std::move(as), p)}
    {}
    using Ptr = std::shared_ptr<Call>;

    const Apply::Ptr subrCall;
};


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
    Subroutine(std::string_view nm, std::vector<std::string> ps, Statement::Ptr b, Position p)
        : Node{NodeKind::Subroutine, p}, 
          name{nm},
          parameters{std::move(ps)},
          body{b}
    {}
    using Ptr = std::shared_ptr<Subroutine>;

    const std::string name = "";               //<! անուն
    const std::vector<std::string> parameters; //<! պարամետրեր
    const Statement::Ptr body;                 //<! մարմին
};


//! @brief Ծրագիր
class Program : public Node {
public:
    explicit Program(std::string_view fn, std::vector<Subroutine::Ptr> m, Position p)
        : Node{NodeKind::Program, p},
          filename{fn}, members{std::move(m)}
    {}
    using Ptr = std::shared_ptr<Program>;

    const std::string filename;                 //!< անունը
    const std::vector<Subroutine::Ptr> members; //!< ենթածրագրերի ցուցակը
};

} // basic
