
#include "aslisp.hxx"

#include <map>
#include <sstream>
#include <string>

namespace basic {
///
std::map<Operation, std::string> mnemonic{
    { Operation::None, "?" },
    { Operation::Add,  "ADD" },
    { Operation::Sub,  "SUB" },
    { Operation::Mul,  "MUL" },
    { Operation::Div,  "DIV" },
    { Operation::Mod,  "MOD" },
    { Operation::Pow,  "POW" },
    { Operation::Eq,   "EQ" },
    { Operation::Ne,   "NE" },
    { Operation::Gt,   "GT" },
    { Operation::Ge,   "GE" },
    { Operation::Lt,   "LT" },
    { Operation::Le,   "LE" },
    { Operation::And,  "AND" },
    { Operation::Or,   "OR" },
    { Operation::Not,  "NOT" },
    { Operation::Conc, "CONC" }
};

///
bool Lisper::asLisp(AstNode* node)
{
    asLispAstNode(node);
    return true;
}

///
void Lisper::asLispNumber(Number* node)
{
    ooo << "(basic-number " << node->value << ")";
}

///
void Lisper::asLispText(Text* node)
{
    ooo << "(basic-text \"" << node->value << "\")";
}

///
void Lisper::asLispVariable(Variable* node)
{
    ooo << "(basic-variable \"" << node->name << "\")";
}

///
void Lisper::asLispUnary(Unary* node)
{
    ooo << "(basic-unary \"" << mnemonic[node->opcode] << "\" ";
    asLispAstNode(node->subexpr);
    ooo << ")";
}

///
void Lisper::asLispBinary(Binary* node)
{
    ooo << "(basic-binary \"" << mnemonic[node->opcode] << "\"";
    ++indent;
    asLispAstNode(node->subexpro);
    asLispAstNode(node->subexpri);
    --indent;
    ooo << ")";
}

///
void Lisper::asLispApply(Apply* node)
{
    ooo << "(basic-apply \"" << node->procptr->name << "\"";
    ++indent;
    for (auto e : node->arguments)
        asLispAstNode(e);
    --indent;
    ooo << ")";
}

///
void Lisper::asLispLet(Let* node)
{
    ooo << "(basic-let (basic-variable \""
        << node->varptr->name << "\") ";
    ++indent;
    asLispAstNode(node->expr);
    --indent;
    ooo << ")";
}

///
void Lisper::asLispInput(Input* node)
{
    ooo << "(basic-input (basic-variable \"" << node->prompt 
        << "\") \"" << node->varptr->name << "\")";
}

///
void Lisper::asLispPrint(Print* node)
{
    ooo << "(basic-print";
    ++indent;
    asLispAstNode(node->expr);
    --indent;
    ooo << ")";
}

///
void Lisper::asLispIf(If* node)
{
    ooo << "(basic-if";
    ++indent;
    asLispAstNode(node->condition);
    asLispAstNode(node->decision);
    if (nullptr != node->alternative)
        asLispAstNode(node->alternative);
    ooo << ")";
    --indent;
}

///
void Lisper::asLispWhile(While* node)
{
    ooo << "(basic-while";
    ++indent;
    asLispAstNode(node->condition);
    asLispAstNode(node->body);
    ooo << ")";
    --indent;
}

///
void Lisper::asLispFor(For* node)
{
    ooo << "(basic-for";
    ++indent;
    asLispAstNode(node->parameter);
    asLispAstNode(node->begin);
    asLispAstNode(node->end);
    asLispAstNode(node->step);
    asLispAstNode(node->body);
    ooo << ")";
    --indent;
}

///
void Lisper::asLispCall(Call* node)
{
    ooo << "(basic-call \"" << (node->subrcall->procptr->name) << "\"";
    ++indent;
    for (auto e : node->subrcall->arguments)
        asLispAstNode(e);
    ooo << ")";
    --indent;
}

///
void Lisper::asLispSequence(Sequence* node)
{
    ooo << "(basic-sequence";
    ++indent;
    for (auto ei : node->items)
        asLispAstNode(ei);
    ooo << ")";
    --indent;
}

///
void Lisper::asLispSubroutine(Subroutine* node)
{
    ooo << "(basic-subroutine \"" << node->name << "\"";
    ++indent;
    std::string parlis = "";
    for (auto& ip : node->parameters) {
        parlis.append("\"");
        parlis.append(ip);
        parlis.append("\" ");
    }
    if (!parlis.empty())
        parlis.pop_back();
    ooo << std::endl << std::string(2 * indent, ' ') << "'(" << parlis << ")";
    asLispAstNode(node->body);
    ooo << ")";
    --indent;
}

///
void Lisper::asLispProgram(Program* node)
{
    ooo << "(basic-program \"" << node->filename << "\"";
    ++indent;
    for (auto si : node->members)
        if (!si->isBuiltIn)
            asLispAstNode(si);
    --indent;
    ooo << ")" << std::endl;
}

void Lisper::asLispAstNode(AstNode* node)
{
    if (nullptr == node)
        return;

    ooo << std::endl << std::string(2 * indent, ' ');

    switch (node->kind) {
        case NodeKind::Number:
            asLispNumber(dynamic_cast<Number*>(node));
            break;
        case NodeKind::Text:
            asLispText(dynamic_cast<Text*>(node));
            break;
        case NodeKind::Variable:
            asLispVariable(dynamic_cast<Variable*>(node));
            break;
        case NodeKind::Unary:
            asLispUnary(dynamic_cast<Unary*>(node));
            break;
        case NodeKind::Binary:
            asLispBinary(dynamic_cast<Binary*>(node));
            break;
        case NodeKind::Apply:
            asLispApply(dynamic_cast<Apply*>(node));
            break;
        case NodeKind::Sequence:
            asLispSequence(dynamic_cast<Sequence*>(node));
            break;
        case NodeKind::Input:
            asLispInput(dynamic_cast<Input*>(node));
            break;
        case NodeKind::Print:
            asLispPrint(dynamic_cast<Print*>(node));
            break;
        case NodeKind::Let:
            asLispLet(dynamic_cast<Let*>(node));
            break;
        case NodeKind::If:
            asLispIf(dynamic_cast<If*>(node));
            break;
        case NodeKind::While:
            asLispWhile(dynamic_cast<While*>(node));
            break;
        case NodeKind::For:
            asLispFor(dynamic_cast<For*>(node));
            break;
        case NodeKind::Call:
            asLispCall(dynamic_cast<Call*>(node));
            break;
        case NodeKind::Subroutine:
            asLispSubroutine(dynamic_cast<Subroutine*>(node));
            break;
        case NodeKind::Program:
            asLispProgram(dynamic_cast<Program*>(node));
            break;
        default:
            break;
    }
}
} // basic
