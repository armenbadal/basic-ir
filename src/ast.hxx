#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

//
namespace basic {

// Աբստրակտ քերականական ծառի հանգույցի տեսակը
enum class NodeKind : int {
    Empty,      // ոչինչ
    Boolean,    // տրամաբանական հաստատուն
    Number,     // թվային հաստատուն
    Text,       // տեքստային հաստատուն
    Variable,   // փոփոխականի հղում
    Unary,      // ունար գործողություն
    Binary,     // բինար գործողություն
    Apply,      // ֆունկցիայի կիրառում
    Array,      // զանգվածի տարրեր
    Sequence,   // հրամանների հաջորդում
    Dim,        // զանգվածի սահմանում
    Input,      // տվյալների ներմուծում
    Print,      // տվյալների արտածում
    Let,        // վերագրում
    If,         // լրիվ ճյուղավորում
    IfThen,     // կարճ ճյուղավորում
    While,      // պայմանով կրկնություն
    For,        // պարամետրով կրկնություն
    Call,       // պրոցեդուրայի կանչ
    Subroutine, // ենթածրագիր
    Program     // ծրագիր
};

// Թոքենի դիրքը տեքստում (հիմա՝ տողը)
using Position = unsigned int;

using NodeId = unsigned int;

// Քերականական ծառի հանգույցի բազային տիպը։
//
// Ծառի հանգույցների բոլոր տեսակներն այս տիպի ընդլայնում են։
class Node {
public:
    Node() = default;
    Node(NodeKind k, Position l)
        : kind{k}
        , line{l}
        , _id{_index++}
    {}
    virtual ~Node() = default;

    virtual NodeId id() const noexcept
    {
        return _id;
    }

    using Ptr = std::shared_ptr<Node>;

    const NodeKind kind{NodeKind::Empty}; // հանգույցի տեսակը
    const Position line{0};               // տողի համարը

private:
    NodeId _id{0};
    inline static NodeId _index{0};
};

template<typename P, typename... Args>
std::shared_ptr<P> node(Args&&... args)
{
    return std::make_shared<P>(std::forward<Args>(args)...);
}


// Արտահայտություն
class Expression : public Node {
public:
    Expression(NodeKind k, Position p)
        : Node{k, p}
    {}

    using Ptr = std::shared_ptr<Expression>;
};


// Տրամաբանական հաստատուն
class Boolean : public Expression {
public:
    Boolean(bool value, Position pos)
        : Expression{NodeKind::Boolean, pos}
        , _value{value}
    {}
    using Ptr = std::shared_ptr<Boolean>;

    const bool _value{false};
};


// Թվային հաստատուն
class Number : public Expression {
public:
    Number(double value, Position pos)
        : Expression{NodeKind::Number, pos}
        ,  _value{value}
    {}
    using Ptr = std::shared_ptr<Number>;

    const double _value{0.0};
};


// Տեքստային հաստատուն
class Text : public Expression {
public:
    Text(std::string_view value, Position pos)
        : Expression{NodeKind::Text, pos}
        , _value{value}
    {}
    using Ptr = std::shared_ptr<Text>;

    const std::string _value;
};
  

// Փոփոխական
class Variable : public Expression {
public:
    Variable(std::string_view name, Position pos)
        : Expression{NodeKind::Variable, pos}
        , _name{name}
    {}
    using Ptr = std::shared_ptr<Variable>;

    const std::string _name; // փոփոխականի անունը
};


// Գործողությունների անունները
enum class Operation {
    None, // անորոշ
    Add,  // գումարում
    Sub,  // հանում
    Mul,  // բազմապատկում
    Div,  // բաժանում
    Mod,  // մնացորդ
    Quot, // քանորդ
    Pow,  // աստիճան
    Eq,   // հավասար է
    Ne,   // հավասար չէ
    Gt,   // մեծ է
    Ge,   // մեծ է կամ հավասար
    Lt,   // փոքր է
    Le,   // փոքր է կամ հավասար
    And,  // ԵՎ (կոնյունկցիա)
    Or,   // ԿԱՄ (դիզյունկցիա)
    Not,  // ՈՉ (ժխտում)
    Conc,  // տեքստերի կցում
    Index  // ինդեքսավորում
};

// Ունար գործողություն
class Unary : public Expression {
public:
    Unary(Operation operation, Expression::Ptr operand, Position pos)
        : Expression{NodeKind::Unary, pos}
        , _operation{operation}
        , _operand{std::move(operand)}
    {}
    using Ptr = std::shared_ptr<Unary>;

    const Operation _operation;      // գործողության կոդը
    const Expression::Ptr _operand;  // օպերանդը
};


// Բինար գործողություն
class Binary : public Expression {
public:
    Binary(Operation operation, Expression::Ptr left, 
           Expression::Ptr right, Position pos)
        : Expression{NodeKind::Binary, pos}
        , _operation{operation}
        , _left{std::move(left)}
        , _right{std::move(right)}
    {}
    using Ptr = std::shared_ptr<Binary>;

    const Operation _operation;    // գործողության կոդը
    const Expression::Ptr _left;  // ձախ օպերանդը
    const Expression::Ptr _right; // աջ օպերանդը
};

// Ֆունկցիայի կանչ (կիրառում)
class Apply : public Expression {
public:
    Apply(std::string_view callee, std::vector<Expression::Ptr> arguments, Position pos)
        : Expression{NodeKind::Apply, pos}
        , _callee{callee}
        , _arguments{std::move(arguments)}
    {}
    using Ptr = std::shared_ptr<Apply>;

    const std::string _callee;                     // կանչվող ենթածրագիրը
    const std::vector<Expression::Ptr> _arguments; // արգումենտները
};

// Զանգվածի թվարկված տարրեր
class Array : public Expression {
public:
    Array(std::vector<Expression::Ptr> elements, Position pos)
        : Expression{NodeKind::Array, pos}
        , _elements{std::move(elements)}
    {}
    using Ptr = std::shared_ptr<Array>;

    const std::vector<Expression::Ptr> _elements; // տարրերը
};


// Ղեկավարող կառուցվածք (հրաման)
class Statement : public Node {
public:
    Statement(NodeKind k, Position p)
        : Node{k, p}
    {}
    using Ptr = std::shared_ptr<Statement>;
};


// Հրամանների շարք (հաջորդականություն)
class Sequence : public Statement {
public:
    Sequence(std::vector<Statement::Ptr> items, Position pos) 
        : Statement{NodeKind::Sequence, pos}
        , _items{std::move(items)}
    {}
    using Ptr = std::shared_ptr<Sequence>;

    const std::vector<Statement::Ptr> _items;
};

// Զանգվածի սահմանում
class Dim : public Statement {
public:
    Dim(std::string_view name, Expression::Ptr size, Position pos)
        : Statement{NodeKind::Dim, pos}
        , _name{name}, _size{std::move(size)}
    {}
    using Ptr = std::shared_ptr<Dim>;

    const std::string _name;     // անունը
    const Expression::Ptr _size; // չափը
};

// Տվյալների ներմուծում
class Input : public Statement {
public:
    Input(Variable::Ptr variable, Position pos)
        : Statement{NodeKind::Input, pos}
        , _variable{std::move(variable)}
    {}
    using Ptr = std::shared_ptr<Input>;

    const Variable::Ptr _variable;   // ներմուծվող փոփոխական
};


// Տվյալների արտածում
class Print : public Statement {
public:
    Print(Expression::Ptr expr, Position pos)
        : Statement{NodeKind::Print, pos}
        , _expr{std::move(expr)}
    {}
    using Ptr = std::shared_ptr<Print>;

    const Expression::Ptr _expr; // արտածվող արտահայտությունը
};


// Վերագրում (միաժամանակ՝ փոփոխականի սահմանում)
class Let : public Statement {
public:
    Let(Variable::Ptr variable, Expression::Ptr expr, Position pos)
        : Statement{NodeKind::Let, pos}
        , _variable{std::move(variable)}
        , _expr{std::move(expr)}
    {}
    using Ptr = std::shared_ptr<Let>;

    const Variable::Ptr _variable;  // փոփոխականը
    const Expression::Ptr _expr; // արժեքը
};


// Ճյուղավորում
class If : public Statement {
public:
    class IfThen : public Statement {
    public:
        IfThen(Expression::Ptr condition, Statement::Ptr decision, Position pos)
            : Statement{NodeKind::IfThen, pos}
            , _condition{condition}
            , _decision{decision}
        {}
        using Ptr = std::shared_ptr<IfThen>;

        const Expression::Ptr _condition; // պայման
        const Statement::Ptr _decision;   // որոշում, then
    };

    If(std::vector<IfThen::Ptr> branches,
       Statement::Ptr alternative, Position pos)
        : Statement{NodeKind::If, pos}
        , _branches{std::move(branches)}
        , _alternative{std::move(alternative)}
    {}
    using Ptr = std::shared_ptr<If>;

    const std::vector<IfThen::Ptr> _branches;  // ճյուղեր
    const Statement::Ptr _alternative; // else ճյուղը
};


// Նախապայմանով ցիկլ
class While : public Statement {
public:
    While(Expression::Ptr condition, Statement::Ptr body, Position pos)
        : Statement{NodeKind::While, pos}
        , _condition{std::move(condition)}
        , _body{std::move(body)}
    {}
    using Ptr = std::shared_ptr<While>;

    const Expression::Ptr _condition; // կրկնման պայման
    const Statement::Ptr _body;       // ցիկլի մարմինը
};


// Պարամետրով ցիկլ
class For : public Statement {
public:
    For(Variable::Ptr parameter, Expression::Ptr begin, Expression::Ptr end, 
        Number::Ptr step, Statement::Ptr body, Position pos)
        : Statement{NodeKind::For, pos}
        , _parameter{std::move(parameter)}
        , _begin{std::move(begin)}
        , _end{std::move(end)}
        , _step{std::move(step)}
        , _body{std::move(body)}
    {}
    using Ptr = std::shared_ptr<For>;

    const Variable::Ptr _parameter; // ցիկլի պարամետրը
    const Expression::Ptr _begin;   // պարամետրի սկզբնակական արժեքը
    const Expression::Ptr _end;     // պարամետրի սահմանային արժեքը
    const Number::Ptr _step;        // պարամետրի փոփոխման քայլը
    const Statement::Ptr _body;     // ցիկլի մարմինը
};


// Ենթածրագրի կանչ
class Call : public Statement {
public:
    Call(std::string_view callee, std::vector<Expression::Ptr> arguments, Position pos)
        : Statement{NodeKind::Call, pos}
        , _subrCall{node<Apply>(callee, std::move(arguments), pos)}
    {}
    using Ptr = std::shared_ptr<Call>;

    const Apply::Ptr _subrCall;
};


// Ենթածրագիր
//
// Ենթածրագիրը օգտագործվում է և՛ որպես պրոցեդուրա, և՛ որպես
// ֆունկցիա։ Դրա վերադարձրած արժեքի տիպը որոշվում է անվան
// կառուցվածքով, ինչպես փոփոխականներինը։ Ենթածրագիրը 
// ֆունկցիա է, եթե նրա մարմնում է անվանը արժեք վերագրող
// LET հրաման։ Այդ դեպքում hasValue անդամի արժեքը
// դրվում է true։
class Subroutine : public Node {
public:
    Subroutine(std::string_view name, 
               std::vector<Variable::Ptr> parameters,
               Statement::Ptr body,
               Position pos)
        : Node{NodeKind::Subroutine, pos}
        , _name{name}
        , _parameters{std::move(parameters)}
        , _body{std::move(body)}
    {}
    using Ptr = std::shared_ptr<Subroutine>;

    const std::string _name = "";                 //<! անուն
    const std::vector<Variable::Ptr> _parameters; //<! պարամետրեր
    const Statement::Ptr _body;                   //<! մարմին
};


// Ծրագիր
class Program : public Node {
public:
    Program(std::vector<Subroutine::Ptr> subroutines, Position pos)
        : Node{NodeKind::Program, pos}
        , _subroutines{std::move(subroutines)}
    {}
    using Ptr = std::shared_ptr<Program>;

    const std::vector<Subroutine::Ptr> _subroutines; // ենթածրագրերի ցուցակը
};

} // basic
