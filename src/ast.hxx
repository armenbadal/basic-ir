
#ifndef AST_HXX
#define AST_HXX

#include <list>
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
    AstNode();
    virtual ~AstNode() = default;

    static void deleteAllocatedNodes();

public:
    NodeKind kind = NodeKind::Empty;
    unsigned int line = 0;

private:
    // բոլոր դինամիկ ստեղծված հանգույցների հասցեները
    static std::list<AstNode*> allocatedNodes;
};

// Տվյալների տիպերը։ Void-ն օգտագործվում է
// արժեք չվերադարձնող ենթածրագրերի հետ աշխատելիս։
enum class Type : char {
    Void = 'V',   // արժեքի բացակայություն
    Number = 'N', // թվային արժեք
    Text = 'T'    // տեքստային արժեք
};

//
Type typeOf(const std::string& nm);

// Արտահայտություն։
class Expression : public AstNode {
public:
    Type type = Type::Void;
};

// @brief Թվային հաստատուն
class Number : public Expression {
public:
    double value = 0.0;

public:
    Number(double vl);
};

// @brief Տեքստային հաստատուն
class Text : public Expression {
public:
    std::string value = "";

public:
    Text(const std::string& vl);
};

// @brief Փոփոխական
//
// Փոփոխականի տիպը որոշվում է նրա անվան կառուցվածքով։
// Եթե այն ավարտվում է '$' նիշով, ապա փոփոխականի տիպը
// @c TEXT է, հակառակ դեպքում՝ @c REAL է։
class Variable : public Expression {
public:
    std::string name = "";

public:
    Variable(const std::string& nm);
};

// Գործողությունների անունները
enum class Operation {
    None, // անորոշ
    Add,  // գումարում
    Sub,  // հանում
    Mul,  // բազմապատկում
    Div,  // բաժանում
    Mod,  // ամբողջ բաժանում
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
    Conc  // տեքստերի կցում
};

// Գործողության տեքստային անունը
std::string toString(Operation opc);

// Ունար գործողություն
class Unary : public Expression {
public:
    Operation opcode = Operation::None;
    Expression* subexpr = nullptr;

public:
    Unary(Operation op, Expression* ex);
};

// Բինար գործողություն
class Binary : public Expression {
public:
    Operation opcode = Operation::None;
    Expression* subexpro = nullptr;
    Expression* subexpri = nullptr;

public:
    Binary(Operation op, Expression* exo, Expression* exi);
};

class Subroutine;

// Ֆունկցիայի կանչ (կիրառում)
class Apply : public Expression {
public:
    Subroutine* procptr = nullptr;
    std::vector<Expression*> arguments;

public:
    Apply(Subroutine* sp, const std::vector<Expression*>& ags);
};

// Ղեկավարող կառուցվածք (հրաման)
class Statement : public AstNode {
};

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
    Input(Variable* vp);
};

// Տվյալների արտածում
class Print : public Statement {
public:
    Expression* expr = nullptr;

public:
    Print(Expression* ex);
};

// Վերագրում (միաժամանակ՝ փոփոխականի սահմանում)
class Let : public Statement {
public:
    Variable* varptr = nullptr;
    Expression* expr = nullptr;

public:
    Let(Variable* vp, Expression* ex);
};

// Ճյուղավորում
class If : public Statement {
public:
    Expression* condition = nullptr;
    Statement* decision = nullptr;
    Statement* alternative = nullptr;

public:
    If(Expression* co, Statement* de, Statement* al = nullptr);
};

// Նախապայմանով ցիկլ
class While : public Statement {
public:
    Expression* condition = nullptr;
    Statement* body = nullptr;

public:
    While(Expression* co, Statement* bo);
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
    For(Variable* pr, Expression* be, Expression* en, Expression* st, Statement* bo);
};

// Ենթածրագրի կանչ
class Call : public Statement {
public:
    Apply* subrcall = nullptr;

public:
    Call(Subroutine* sp, const std::vector<Expression*> as);
};

// Ենթածրագիր
//
// Ենթածրագիրը օգտագործվում է և՛ որպես պրոցեդուրա, և՛ որպես
// ֆունկցիա։ Դրա վերադարձրած արժեքի տիպը որոշվում է անվան
// կառուցվածքով, ինչպես փոփոխականներինը։ Ենթածրագիրը 
// ֆունկցիա է, եթե նրա մարմնում է անվանը արժեք վերագրող
// LET հրաման։ Այդ դեպքում @c hasvalue անդամի արժեքը
// դրվում է @c true ։
class Subroutine : public AstNode {
public:
    std::string name = ""; // անուն
    std::vector<std::string> parameters; // պարամետրեր
    std::vector<Variable*> locals; // լոկալ փոփոխականներ
    Statement* body = nullptr; // մարմին
    bool hasValue = false; // վերադարձնո՞ւմ է արժեքի

public:
    Subroutine(const std::string& nm, const std::vector<std::string>& ps, Statement* bo);
};

// Ծրագիր
class Program : public AstNode {
public:
    std::string filename = "";
    std::vector<Subroutine*> members;

public:
    Program(const std::string& fn);
};

} // basic

#endif // AST_HXX
