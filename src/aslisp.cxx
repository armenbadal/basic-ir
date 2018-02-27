
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
int Lisper::convertNumber(Number* node)
{
    ooo << "(basic-number " << node->value << ")";
    return 0;
}

///
int Lisper::convertText(Text* node)
{
    ooo << "(basic-text \"" << node->value << "\")";
    return 0;
}

///
int Lisper::convertVariable(Variable* node)
{
    ooo << "(basic-variable \"" << node->name << "\")";
    return 0;
}

///
int Lisper::convertUnary(Unary* node)
{
    ooo << "(basic-unary \"" << mnemonic[node->opcode] << "\" ";
    convertAstNode(node->subexpr);
    ooo << ")";
    return 0;
}

///
int Lisper::convertBinary(Binary* node)
{
    ooo << "(basic-binary \"" << mnemonic[node->opcode] << "\"";
    ++indent;
    convertAstNode(node->subexpro);
    convertAstNode(node->subexpri);
    --indent;
    ooo << ")";
    return 0;
}

///
int Lisper::convertApply(Apply* node)
{
    ooo << "(basic-apply \"" << node->procptr->name << "\"";
    ++indent;
    for (auto e : node->arguments)
        convertAstNode(e);
    --indent;
    ooo << ")";
    return 0;
}

///
int Lisper::convertLet(Let* node)
{
    ooo << "(basic-let (basic-variable \"" << node->varptr->name << "\") ";
    ++indent;
    convertAstNode(node->expr);
    --indent;
    ooo << ")";
    return 0;
}

///
int Lisper::convertInput(Input* node)
{
    ooo << "(basic-input (basic-variable \"" << node->prompt << "\") \"" << node->varptr->name << "\")";
    return 0;
}

///
int Lisper::convertPrint(Print* node)
{
    ooo << "(basic-print";
    ++indent;
    convertAstNode(node->expr);
    --indent;
    ooo << ")";
    return 0;
}

///
int Lisper::convertIf(If* node)
{
    ooo << "(basic-if";
    ++indent;
    convertAstNode(node->condition);
    convertAstNode(node->decision);
    if (nullptr != node->alternative)
        convertAstNode(node->alternative);
    ooo << ")";
    --indent;
    return 0;
}

///
int Lisper::convertWhile(While* node)
{
    ooo << "(basic-while";
    ++indent;
    convertAstNode(node->condition);
    convertAstNode(node->body);
    ooo << ")";
    --indent;
    return 0;
}

///
int Lisper::convertFor(For* node)
{
    ooo << "(basic-for";
    ++indent;
    convertAstNode(node->parameter);
    convertAstNode(node->begin);
    convertAstNode(node->end);
    convertAstNode(node->step);
    convertAstNode(node->body);
    ooo << ")";
    --indent;
    return 0;
}

///
int Lisper::convertCall(Call* node)
{
    ooo << "(basic-call \"" << (node->subrcall->procptr->name) << "\"";
    ++indent;
    for (auto e : node->subrcall->arguments)
        convertAstNode(e);
    ooo << ")";
    --indent;
    return 0;
}

///
int Lisper::convertSequence(Sequence* node)
{
    ooo << "(basic-sequence";
    ++indent;
    for (auto ei : node->items)
        convertAstNode(ei);
    ooo << ")";
    --indent;
    return 0;
}

///
int Lisper::convertSubroutine(Subroutine* node)
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
    convertAstNode(node->body);
    ooo << ")";
    --indent;
    return 0;
}

///
int Lisper::convertProgram(Program* node)
{
    ooo << "(basic-program \"" << node->filename << "\"";
    ++indent;
    for (auto si : node->members)
        if (!si->isBuiltIn)
            convertSubroutine(si);
    --indent;
    ooo << ")" << std::endl;
    return 0;
}

int Lisper::convertAstNode(AstNode* node)
{
    space(); 
    return Converter::convertAstNode(node);
}

void Lisper::space()
{
    ooo << std::endl << std::string(2 * indent, ' ');
}
} // basic
