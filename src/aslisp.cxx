
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
void Lisper::compileNumber(Number* node)
{
    ooo << "(basic-number " << node->value << ")";
}

///
void Lisper::compileText(Text* node)
{
    ooo << "(basic-text \"" << node->value << "\")";
}

///
void Lisper::compileVariable(Variable* node)
{
    ooo << "(basic-variable \"" << node->name << "\")";
}

///
void Lisper::compileUnary(Unary* node)
{
    ooo << "(basic-unary \"" << mnemonic[node->opcode] << "\" ";
    compileAstNode(node->subexpr);
    ooo << ")";
}

///
void Lisper::compileBinary(Binary* node)
{
    ooo << "(basic-binary \"" << mnemonic[node->opcode] << "\"";
    ++indent;
    compileAstNode(node->subexpro);
    compileAstNode(node->subexpri);
    --indent;
    ooo << ")";
}

///
void Lisper::compileApply(Apply* node)
{
    ooo << "(basic-apply \"" << node->procptr->name << "\"";
    ++indent;
    for (auto e : node->arguments)
        compileAstNode(e);
    --indent;
    ooo << ")";
}

///
void Lisper::compileLet(Let* node)
{
    ooo << "(basic-let (basic-variable \"" << node->varptr->name << "\") ";
    ++indent;
    compileAstNode(node->expr);
    --indent;
    ooo << ")";
}

///
void Lisper::compileInput(Input* node)
{
    ooo << "(basic-input (basic-variable \"" << node->prompt << "\") \"" << node->varptr->name << "\")";
    return 0;
}

///
void Lisper::compilePrint(Print* node)
{
    ooo << "(basic-print";
    ++indent;
    compileAstNode(node->expr);
    --indent;
    ooo << ")";
}

///
void Lisper::compileIf(If* node)
{
    ooo << "(basic-if";
    ++indent;
    compileAstNode(node->condition);
    compileAstNode(node->decision);
    if (nullptr != node->alternative)
        compileAstNode(node->alternative);
    ooo << ")";
    --indent;
}

///
void Lisper::compileWhile(While* node)
{
    ooo << "(basic-while";
    ++indent;
    compileAstNode(node->condition);
    compileAstNode(node->body);
    ooo << ")";
    --indent;
}

///
void Lisper::compileFor(For* node)
{
    ooo << "(basic-for";
    ++indent;
    compileAstNode(node->parameter);
    compileAstNode(node->begin);
    compileAstNode(node->end);
    compileAstNode(node->step);
    compileAstNode(node->body);
    ooo << ")";
    --indent;
}

///
void Lisper::compileCall(Call* node)
{
    ooo << "(basic-call \"" << (node->subrcall->procptr->name) << "\"";
    ++indent;
    for (auto e : node->subrcall->arguments)
        compileAstNode(e);
    ooo << ")";
    --indent;
    return 0;
}

///
void Lisper::compileSequence(Sequence* node)
{
    ooo << "(basic-sequence";
    ++indent;
    for (auto ei : node->items)
        compileAstNode(ei);
    ooo << ")";
    --indent;
}

///
void Lisper::compileSubroutine(Subroutine* node)
{
    space();
    ooo << "(basic-subroutine \"" << node->name << "\"";
    ++indent;
    space();
    std::string parlis = "";
    for (auto& ip : node->parameters) {
        parlis.append("\"");
        parlis.append(ip);
        parlis.append("\" ");
    }
    if (!parlis.empty())
        parlis.pop_back();
    ooo << "'(" << parlis << ")";
    compileAstNode(node->body);
    ooo << ")";
    --indent;
}

///
void Lisper::compileProgram(Program* node)
{
    ooo << "(basic-program \"" << node->filename << "\"";
    ++indent;
    for (auto si : node->members)
        if (!si->isBuiltIn)
            compileSubroutine(si);
    --indent;
    ooo << ")" << std::endl;
}

void Lisper::compileAstNode(AstNode* node)
{
    space(); 
    Converter::compileAstNode(node);
}

void Lisper::space()
{
    ooo << std::endl << std::string(2 * indent, ' ');
}
} // basic
