
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
void Lisper::visitNumber(Number* node)
{
    ooo << "(basic-number " << node->value << ")";
}

///
void Lisper::visitText(Text* node)
{
    ooo << "(basic-text \"" << node->value << "\")";
}

///
void Lisper::visitVariable(Variable* node)
{
    ooo << "(basic-variable \"" << node->name << "\")";
}

///
void Lisper::visitUnary(Unary* node)
{
    ooo << "(basic-unary \"" << mnemonic[node->opcode] << "\" ";
    visitAstNode(node->subexpr);
    ooo << ")";
}

///
void Lisper::visitBinary(Binary* node)
{
    ooo << "(basic-binary \"" << mnemonic[node->opcode] << "\"";
    ++indent;
    visitAstNode(node->subexpro);
    visitAstNode(node->subexpri);
    --indent;
    ooo << ")";
}

///
void Lisper::visitApply(Apply* node)
{
    ooo << "(basic-apply \"" << node->procptr->name << "\"";
    ++indent;
    for (auto e : node->arguments)
        visitAstNode(e);
    --indent;
    ooo << ")";
}

///
void Lisper::visitLet(Let* node)
{
    ooo << "(basic-let (basic-variable \""
        << node->varptr->name << "\") ";
    ++indent;
    visitAstNode(node->expr);
    --indent;
    ooo << ")";
}

///
void Lisper::visitInput(Input* node)
{
    ooo << "(basic-input (basic-variable \"" << node->prompt 
        << "\") \"" << node->varptr->name << "\")";
}

///
void Lisper::visitPrint(Print* node)
{
    ooo << "(basic-print";
    ++indent;
    visitAstNode(node->expr);
    --indent;
    ooo << ")";
}

///
void Lisper::visitIf(If* node)
{
    ooo << "(basic-if";
    ++indent;
    visitAstNode(node->condition);
    visitAstNode(node->decision);
    if (nullptr != node->alternative)
        visitAstNode(node->alternative);
    ooo << ")";
    --indent;
}

///
void Lisper::visitWhile(While* node)
{
    ooo << "(basic-while";
    ++indent;
    visitAstNode(node->condition);
    visitAstNode(node->body);
    ooo << ")";
    --indent;
}

///
void Lisper::visitFor(For* node)
{
    ooo << "(basic-for";
    ++indent;
    visitAstNode(node->parameter);
    visitAstNode(node->begin);
    visitAstNode(node->end);
    visitAstNode(node->step);
    visitAstNode(node->body);
    ooo << ")";
    --indent;
}

///
void Lisper::visitCall(Call* node)
{
    ooo << "(basic-call \"" << (node->subrcall->procptr->name) << "\"";
    ++indent;
    for (auto e : node->subrcall->arguments)
        visitAstNode(e);
    ooo << ")";
    --indent;
}

///
void Lisper::visitSequence(Sequence* node)
{
    ooo << "(basic-sequence";
    ++indent;
    for (auto ei : node->items)
        visitAstNode(ei);
    ooo << ")";
    --indent;
}

///
void Lisper::visitSubroutine(Subroutine* node)
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
    ooo << std::endl << std::string(2 * indent, ' ')
        << "'(" << parlis << ")";
    visitAstNode(node->body);
    ooo << ")";
    --indent;
}

///
void Lisper::visitProgram(Program* node)
{
    ooo << "(basic-program \"" << node->filename << "\"";
    ++indent;
    for (auto si : node->members)
        if (!si->isBuiltIn)
            visitAstNode(si);
    --indent;
    ooo << ")" << std::endl;
}

void Lisper::visitAstNode(AstNode* node)
{
    if (nullptr == node)
        return;

    ooo << std::endl << std::string(2 * indent, ' ');

    switch (node->kind) {
        case NodeKind::Number:
            visitNumber(dynamic_cast<Number*>(node));
            break;
        case NodeKind::Text:
            visitText(dynamic_cast<Text*>(node));
            break;
        case NodeKind::Variable:
            visitVariable(dynamic_cast<Variable*>(node));
            break;
        case NodeKind::Unary:
            visitUnary(dynamic_cast<Unary*>(node));
            break;
        case NodeKind::Binary:
            visitBinary(dynamic_cast<Binary*>(node));
            break;
        case NodeKind::Apply:
            visitApply(dynamic_cast<Apply*>(node));
            break;
        case NodeKind::Sequence:
            visitSequence(dynamic_cast<Sequence*>(node));
            break;
        case NodeKind::Input:
            visitInput(dynamic_cast<Input*>(node));
            break;
        case NodeKind::Print:
            visitPrint(dynamic_cast<Print*>(node));
            break;
        case NodeKind::Let:
            visitLet(dynamic_cast<Let*>(node));
            break;
        case NodeKind::If:
            visitIf(dynamic_cast<If*>(node));
            break;
        case NodeKind::While:
            visitWhile(dynamic_cast<While*>(node));
            break;
        case NodeKind::For:
            visitFor(dynamic_cast<For*>(node));
            break;
        case NodeKind::Call:
            visitCall(dynamic_cast<Call*>(node));
            break;
        case NodeKind::Subroutine:
            visitSubroutine(dynamic_cast<Subroutine*>(node));
            break;
        case NodeKind::Program:
            visitProgram(dynamic_cast<Program*>(node));
            break;
        default:
            break;
    }
}
} // basic
