
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

void AstNode::printKind()
{
    static std::map<NodeKind, std::string> nodeKindString{
        { NodeKind::Empty, "Empty" },
        { NodeKind::Number, "Number" },
        { NodeKind::Text, "Text" },
        { NodeKind::Variable, "Variable" },
        { NodeKind::Unary, "Unary" },
        { NodeKind::Binary, "Binary" },
        { NodeKind::Apply, "Apply" },
        { NodeKind::Sequence, "Sequence" },
        { NodeKind::Input, "Input" },
        { NodeKind::Print, "Print" },
        { NodeKind::Let, "Let" },
        { NodeKind::If, "If" },
        { NodeKind::While, "While" },
        { NodeKind::For, "For" },
        { NodeKind::Call, "Call" },
        { NodeKind::Subroutine, "Subroutine" },
        { NodeKind::Program, "Program" }
    };
    std::cout << nodeKindString[kind] << std::endl;
}

//
AstNode::AstNode()
{
    allocatedNodes.push_front(this);
}

Sequence::Sequence()
{
    kind = NodeKind::Sequence;
}

//
Number::Number(double vl)
    : value{ vl }
{
    kind = NodeKind::Number;
    type = Type::Number;
}

//
Text::Text(const std::string& vl)
    : value{ vl }
{
    kind = NodeKind::Text;
    type = Type::Text;
}

//
Variable::Variable(const std::string& nm)
    : name{ nm }
{
    kind = NodeKind::Variable;
    type = name.back() == '$' ? Type::Text : Type::Number;
}

//
Unary::Unary(Operation op, Expression* ex)
    : opcode{ op }, subexpr{ ex }
{
    kind = NodeKind::Unary;
    type = Type::Number;
}

//
Binary::Binary(Operation op, Expression* exo, Expression* exi)
    : opcode{ op }, subexpro{ exo }, subexpri{ exi }
{
    kind = NodeKind::Binary;
}

//
Apply::Apply(Subroutine* sp, const std::vector<Expression*>& ags)
    : procptr{ sp }, arguments{ ags }
{
    kind = NodeKind::Apply;
}

//
Input::Input(Variable* vp)
    : varptr{ vp }
{
    kind = NodeKind::Input;
}

//
Print::Print(Expression* ex)
    : expr{ ex }
{
    kind = NodeKind::Print;
}

//
Let::Let(Variable* vp, Expression* ex)
    : varptr{ vp }, expr{ ex }
{
    kind = NodeKind::Let;
}

//
If::If(Expression* co, Statement* de, Statement* al)
    : condition{ co }, decision{ de }, alternative{ al }
{
    kind = NodeKind::If;
}

//
While::While(Expression* co, Statement* bo)
    : condition{ co }, body{ bo }
{
    kind = NodeKind::While;
}

//
For::For(Variable* pr, Expression* be, Expression* en, Expression* st, Statement* bo)
    : parameter{ pr }, begin{ be }, end{ en }, step{ st }, body{ bo }
{
    kind = NodeKind::For;
}

//
Call::Call(Subroutine* sp, const std::vector<Expression*> as)
    : subrcall{ new Apply{ sp, as } }
{
    kind = NodeKind::Call;
}

//
Subroutine::Subroutine(const std::string& nm, const std::vector<std::string>& ps, Statement* bo)
    : name{ nm }, parameters{ ps }, body{ bo }
{
    kind = NodeKind::Subroutine;
    locals.push_back(new Variable(name));
    for (auto& ps : parameters)
        locals.push_back(new Variable(ps));
}

//
Program::Program(const std::string& fn)
    : filename{ fn }
{
    kind = NodeKind::Program;
}
} // basic
