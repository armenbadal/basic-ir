
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
    ooo << "(basic-number :value " << node->value << ")";
    return 0;
}

///
int Lisper::convertText(Text* node)
{
    ooo << "(basic-text :value \"" << node->value << "\")";
    return 0;
}

///
int Lisper::convertVariable(Variable* node)
{
    ooo << "(basic-variable :name \"" << node->name << "\")";
    return 0;
}

///
int Lisper::convertUnary(Unary* node)
{
    ooo << "(basic-unary :opcode \"" << mnemonic[node->opcode] << "\" :subexpr ";
    convertAstNode(node->subexpr);
    ooo << ")";
    return 0;
}

///
int Lisper::convertBinary(Binary* node)
{
    ooo << "(basic-binary :opcode \"" << mnemonic[node->opcode] << "\"";
    ++indent;
    space(ooo);
    ooo << ":subexpro ";
    convertAstNode(node->subexpro);
    space(ooo);
    ooo << ":subexpri ";
    convertAstNode(node->subexpri);
    ooo << ")";
    return 0;
}

///
int Lisper::convertApply(Apply* node)
{
    ooo << "(basic-apply :procname \"" << node->procptr->name << "\"";
    ooo << " :arguments (";
    for (auto e : node->arguments) {
        convertAstNode(e);
        ooo << " ";
    }
    ooo << "))";
    return 0;
}

///
int Lisper::convertLet(Let* node)
{
    ooo << "(basic-let :varname " << node->varptr->name;
    ++indent;
    space(ooo);
    ooo << ":expr ";
    convertAstNode(node->expr);
    ooo << ")";
    return 0;
}

///
int Lisper::convertInput(Input* node)
{
    ooo << "(basic-input :varname " << node->varptr->name << ")";
    return 0;
}

///
int Lisper::convertPrint(Print* node)
{
    ooo << "(basic-print :expr ";
    convertAstNode(node->expr);
    ooo << ")";
    return 0;
}

///
int Lisper::convertIf(If* node)
{
    ooo << "(basic-if :condition ";
    convertAstNode(node->condition);
    ooo << " :decision ";
    convertAstNode(node->decision);
    ooo << " :alternative ";
    convertAstNode(node->alternative);
    ooo << ")";
    return 0;
}

///
int Lisper::convertWhile(While* node)
{
    ooo << "(basic-while :condition ";
    convertAstNode(node->condition);
    ooo << " :body ";
    convertAstNode(node->body);
    ooo << ")";
    return 0;
}

///
int Lisper::convertFor(For* node)
{
    ooo << "(basic-for :parameter " << node->parameter;
    ooo << " :begin ";
    convertAstNode(node->begin);
    ooo << " :end ";
    convertAstNode(node->end);
    ooo << " :step ";
    convertAstNode(node->step);
    ooo << " :body ";
    convertAstNode(node->body);
    ooo << ")";
    return 0;
}

///
int Lisper::convertCall(Call* node)
{
    ooo << "(basic-call :procname " << node->subrcall->procptr->name;
    ooo << " :arguments (";
    for (auto e : node->subrcall->arguments) {
        convertAstNode(e);
        ooo << " ";
    }
    ooo << "))";
    return 0;
}

///
int Lisper::convertSequence(Sequence* node)
{
    ooo << "(basic-sequence";
    ++indent;
    space(ooo);
    ooo << ":items (";
    ++indent;
    for (auto ei : node->items) {
        space(ooo);
        convertAstNode(ei);
        ooo << " ";
    }
    --indent;
    --indent;
    space(ooo);
    ooo << "))";
    return 0;
}

///
int Lisper::convertSubroutine(Subroutine* node)
{
    ooo << "(basic-subroutine :name " << node->name;
    ++indent;
    space(ooo);
    ooo << ":parameters (";
    for (auto& ip : node->parameters)
        ooo << ip << " ";
    ooo << ")";
    space(ooo);
    ooo << ":body ";
    convertAstNode(node->body);
    --indent;
    space(ooo);
    ooo << ")";
    --indent;
    return 0;
}

///
int Lisper::convertProgram(Program* node)
{
    ooo << "(basic-program :filename " << node->filename;
    ++indent;
    space(ooo);
    ooo << ":members (";
    for (auto si : node->members) {
        space(ooo);
        convertAstNode(si);
    }
    --indent;
    space(ooo);
    ooo << ")";
    space(ooo);
    ooo << ")" << std::endl;
    return 0;
}

void Lisper::space(std::ostream& ooo)
{
    ooo << std::endl
        << std::string(indent, '\t');
}
} // basic
