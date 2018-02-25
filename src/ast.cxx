
#include "ast.hxx"

#include <iostream>
#include <map>

namespace basic {
//
std::list<AstNode*> AstNode::allocatedNodes;

//
void AstNode::deleteAllocatedNodes()
{
    for (auto e : allocatedNodes)
        delete e;
}

//
std::string toString(Operation opc)
{
    static std::map<Operation, std::string> names{
        { Operation::None, "None" },
        { Operation::Add, "+" },
        { Operation::Sub, "-" },
        { Operation::Mul, "*" },
        { Operation::Div, "/" },
        { Operation::Mod, "\\" },
        { Operation::Pow, "^" },
        { Operation::Eq, "=" },
        { Operation::Ne, "<>" },
        { Operation::Gt, ">" },
        { Operation::Ge, ">=" },
        { Operation::Lt, "<" },
        { Operation::Le, "<=" },
        { Operation::And, "AND" },
        { Operation::Or, "OR" },
        { Operation::Not, "NOT" },
        { Operation::Conc, "&" }
    };
    return names[opc];
}

//
Type typeOf(const std::string& nm)
{
    return nm.back() == '$' ? Type::Text : Type::Number;
}

//
AstNode::AstNode()
{
    allocatedNodes.push_front(this);
}

//
Number::Number(double vl)
    : value(vl)
{
    kind = NodeKind::Number;
    type = Type::Number;
}

//
Text::Text(const std::string& vl)
    : value(vl)
{
    kind = NodeKind::Text;
    type = Type::Text;
}

//
Variable::Variable(const std::string& nm)
    : name(nm)
{
    kind = NodeKind::Variable;
    type = typeOf(name);
}

//
Unary::Unary(Operation op, Expression* ex)
    : opcode(op), subexpr(ex)
{
    kind = NodeKind::Unary;
    type = Type::Number;
}

//
Binary::Binary(Operation op, Expression* exo, Expression* exi)
    : opcode(op), subexpro(exo), subexpri(exi)
{
    kind = NodeKind::Binary;
}

//
Apply::Apply(Subroutine* sp, const std::vector<Expression*>& ags)
    : procptr(sp), arguments(ags)
{
    kind = NodeKind::Apply;
}

//
Sequence::Sequence()
{
    kind = NodeKind::Sequence;
}

//
Input::Input(const std::string& pr, Variable* vp)
    : prompt(pr), varptr(vp)
{
    kind = NodeKind::Input;
}

//
Print::Print(Expression* ex)
    : expr(ex)
{
    kind = NodeKind::Print;
}

//
Let::Let(Variable* vp, Expression* ex)
    : varptr(vp), expr(ex)
{
    kind = NodeKind::Let;
}

//
If::If(Expression* co, Statement* de, Statement* al)
    : condition(co), decision(de), alternative(al)
{
    kind = NodeKind::If;
}

//
While::While(Expression* co, Statement* bo)
    : condition(co), body(bo)
{
    kind = NodeKind::While;
}

//
For::For(Variable* pr, Expression* be, Expression* en, Expression* st, Statement* bo)
    : parameter(pr), begin(be), end(en), step(st), body(bo)
{
    kind = NodeKind::For;
}

//
Call::Call(Subroutine* sp, const std::vector<Expression*> as)
    : subrcall(new Apply(sp, as))
{
    kind = NodeKind::Call;
}

//
Subroutine::Subroutine(const std::string& nm, const std::vector<std::string>& ps, Statement* bo)
    : name(nm), parameters(ps), body(bo)
{
    kind = NodeKind::Subroutine;
    locals.push_back(new Variable(name));
    for (auto& ps : parameters)
        locals.push_back(new Variable(ps));
}

//
Program::Program(const std::string& fn)
    : filename(fn)
{
    kind = NodeKind::Program;
}
} // basic
