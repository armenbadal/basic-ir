
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

//! @brief Տվյալների տիպերը
//!
//! @c Void -ն օգտագործվում է արժեք չվերադարձնող ենթածրագրերի հետ աշխատելիս։
enum class Type : char {
    Void = 'V',   //!< արժեքի բացակայություն
    Number = 'N', //!< թվային արժեք
    Text = 'T'    //!< տեքստային արժեք
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

//! @brief Թվային հաստատուն
class Number : public Expression {
public:
    double value = 0.0;

public:
    Number(double vl);
};

//! @brief Տեքստային հաստատուն
class Text : public Expression {
public:
    std::string value = "";

public:
    Text(const std::string& vl);
};

//! @brief Փոփոխական
class Variable : public Expression {
public:
    std::string name = ""; //!< փոփոխականի անունը

public:
    Variable(const std::string& nm);
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

//! @brief Գործողության տեքստային անունը
std::string toString(Operation opc);

//! @brief Ունար գործողություն
class Unary : public Expression {
public:
    Operation opcode = Operation::None; //!< գործողությոն կոդը
    std::shared_ptr<Expression> subexpr;      //!< օպերանդը

public:
    Unary( Operation op, std::shared_ptr<Expression> ex );
};

//! @brief Բինար գործողություն
class Binary : public Expression {
public:
    Operation opcode = Operation::None; //!< գործողությոն կոդը
    std::shared_ptr<Expression> subexpro;     //!< ձախ օպերանդը
    std::shared_ptr<Expression> subexpri;     //!< աջ օպերանդը

public:
    Binary( Operation op, std::shared_ptr<Expression> exo, std::shared_ptr<Expression> exi );
};

class Subroutine;

//! @brief Ֆունկցիայի կանչ (կիրառում)
class Apply : public Expression {
public:
    std::shared_ptr<Subroutine> procptr;      //!< կանչվող ենթածրագիրը
    std::vector<std::shared_ptr<Expression>> arguments; //!< արգումենտները

public:
    Apply( std::shared_ptr<Subroutine> sp, const std::vector<std::shared_ptr<Expression>>& ags );
};

//! @brief Ղեկավարող կառուցվածք (հրաման)
class Statement : public AstNode {
};

//! @brief Հրամանների շարք (հաջորդականություն)
class Sequence : public Statement {
public:
    std::vector<std::shared_ptr<Statement>> items;

public:
    Sequence();
};

//! @brief Տվյալների ներմուծում
class Input : public Statement {
public:
    std::string prompt = "";    //!< ներմուծման հրավերք
    std::shared_ptr<Variable> varptr; //!< ներմուծվող փոփոխական

public:
    Input( const std::string& pr, std::shared_ptr<Variable> vp );
};

//! @brief Տվյալների արտածում
class Print : public Statement {
public:
    std::shared_ptr<Expression> expr; //!< արտածվող արտահայտությունը

public:
    Print( std::shared_ptr<Expression> ex );
};

//! @brief Վերագրում (միաժամանակ՝ փոփոխականի սահմանում)
class Let : public Statement {
public:
    std::shared_ptr<Variable> varptr; //!< փոփոխականը
    std::shared_ptr<Expression> expr; //!< արժեքը

public:
    Let( std::shared_ptr<Variable> vp, std::shared_ptr<Expression> ex );
};

//! @brief Ճյուղավորում
class If : public Statement {
public:
    std::shared_ptr<Expression> condition;  //!< ճյուղավորման պայման
    std::shared_ptr<Statement> decision;    //!< @c then ճյուղը
    std::shared_ptr<Statement> alternative; //!< @c else ճյուղը

public:
    If( std::shared_ptr<Expression> co, std::shared_ptr<Statement> de, std::shared_ptr<Statement> al = nullptr);
};

//! @brief Նախապայմանով ցիկլ
class While : public Statement {
public:
    std::shared_ptr<Expression> condition; //!< կրկնման պայման
    std::shared_ptr<Statement> body;       //!< ցիկլի մարմինը

public:
    While( std::shared_ptr<Expression> co, std::shared_ptr<Statement> bo );
};

//! @brief Պարամետրով ցիկլ
class For : public Statement {
public:
    std::shared_ptr<Variable> parameter; //!< ցիկլի պարամետրը
    std::shared_ptr<Expression> begin;   //!< պարամետրի սկզբնակական արժեքը
    std::shared_ptr<Expression> end;     //!< պարամետրի սահմանային արժեքը
    std::shared_ptr<Expression> step;    //!< պարամետրի փոփոխման քայլը
    std::shared_ptr<Statement> body;     //!< ցիկլի մարմինը

public:
    For( std::shared_ptr<Variable> pr, std::shared_ptr<Expression> be, 
         std::shared_ptr<Expression> en, std::shared_ptr<Expression> st,
         std::shared_ptr<Statement> bo );
};

//! @brief Ենթածրագրի կանչ
class Call : public Statement {
public:
    std::shared_ptr<Apply> subrcall;

public:
    Call( std::shared_ptr<Subroutine> sp, const std::vector<std::shared_ptr<Expression>>& as );
};

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
    std::vector<std::shared_ptr<Variable>> locals;       //<! լոկալ փոփոխականներ
    std::shared_ptr<Statement> body;           //<! մարմին
    bool hasValue = false;               //<! վերադարձնո՞ւմ է արժեքի

public:
    Subroutine( const std::string& nm, const std::vector<std::string>& ps );
};

//! @brief Ծրագիր
class Program : public AstNode {
public:
    std::string filename = "";        //!< անունը
    std::vector<std::shared_ptr<Subroutine>> members; //!< ենթածրագրերի ցուցակը

public:
    Program( const std::string& fn );
};

} // basic

#endif // AST_HXX
