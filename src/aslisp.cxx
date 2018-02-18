
#include "aslisp.hxx"

#include <map>
#include <sstream>
#include <string>

namespace basic {
///
std::map<Operation, std::string> mnemonic{
    { Operation::Add, "ADD" },
    { Operation::Sub, "SUB" },
    { Operation::Conc, "CONC" },
    { Operation::Mul, "MUL" },
    { Operation::Div, "DIV" },
    { Operation::Mod, "MOD" },
    { Operation::Pow, "POW" },
    { Operation::Eq, "EQ" },
    { Operation::Ne, "NE" },
    { Operation::Gt, "GT" },
    { Operation::Ge, "GE" },
    { Operation::Lt, "LT" },
    { Operation::Le, "LE" },
    { Operation::And, "AND" },
    { Operation::Or, "OR" }
};

///
int Lisper::convertNumber(Number* node, std::ostream& ooo)
{
    ooo << "(basic-number :value " << node->value << ")";
    return 0;
}

///
int Lisper::convertText(Text* node, std::ostream& ooo)
{
    ooo << "(basic-text :value \"" << node->value << "\")";
    return 0;
}

///
int Lisper::convertVariable(Variable* node, std::ostream& ooo)
{
    ooo << "(basic-variable :name \"" << node->name << "\")";
    return 0;
}

///
int Lisper::convertUnary(Unary* node, std::ostream& ooo)
{
    ooo << "(basic-unary :opcode \"" << mnemonic[node->opcode] << "\" :subexpr ";
    convertAstNode(node->subexpr, ooo);
    ooo << ")";
    return 0;
}

///
int Lisper::convertBinary(Binary* node, std::ostream& ooo)
{
    ooo << "(basic-binary :opcode \"" << mnemonic[node->opcode] << "\"";
    ++indent;
    space(ooo);
    ooo << ":subexpro ";
    convertAstNode(node->subexpro, ooo);
    space(ooo);
    ooo << ":subexpri ";
    convertAstNode(node->subexpri, ooo);
    ooo << ")";
    return 0;
}

///
int Lisper::convertApply(Apply* node, std::ostream& ooo)
{
    ooo << "(basic-apply :procname \"" << node->procptr->name << "\"";
    ooo << " :arguments (";
    for (auto e : node->arguments) {
        convertAstNode(e, ooo);
        ooo << " ";
    }
    ooo << "))";
    return 0;
}

///
int Lisper::convertLet(Let* node, std::ostream& ooo)
{
    ooo << "(basic-let :varname " << node->varptr->name;
    ++indent;
    space(ooo);
    ooo << ":expr ";
    convertAstNode(node->expr, ooo);
    ooo << ")";
    return 0;
}

///
int Lisper::convertInput(Input* node, std::ostream& ooo)
{
    ooo << "(basic-input :varname " << node->varptr->name << ")";
    return 0;
}

///
int Lisper::convertPrint(Print* node, std::ostream& ooo)
{
    ooo << "(basic-print :expr ";
    convertAstNode(node->expr, ooo);
    ooo << ")";
    return 0;
}

///
int Lisper::convertIf(If* node, std::ostream& ooo)
{
    ooo << "(basic-if :condition ";
    convertAstNode(node->condition, ooo);
    ooo << " :decision ";
    convertAstNode(node->decision, ooo);
    ooo << " :alternative ";
    convertAstNode(node->alternative, ooo);
    ooo << ")";
    return 0;
}

///
int Lisper::convertWhile(While* node, std::ostream& ooo)
{
    ooo << "(basic-while :condition ";
    convertAstNode(node->condition, ooo);
    ooo << " :body ";
    convertAstNode(node->body, ooo);
    ooo << ")";
    return 0;
}

///
int Lisper::convertFor(For* node, std::ostream& ooo)
{
    ooo << "(basic-for :parameter " << node->parameter;
    ooo << " :begin ";
    convertAstNode(node->begin, ooo);
    ooo << " :end ";
    convertAstNode(node->end, ooo);
    ooo << " :step ";
    convertAstNode(node->step, ooo);
    ooo << " :body ";
    convertAstNode(node->body, ooo);
    ooo << ")";
    return 0;
}

///
int Lisper::convertCall(Call* node, std::ostream& ooo)
{
    ooo << "(basic-call :procname " << node->subrcall->procptr->name;
    ooo << " :arguments (";
    for (auto e : node->subrcall->arguments) {
        convertAstNode(e, ooo);
        ooo << " ";
    }
    ooo << "))";
    return 0;
}

///
int Lisper::convertSequence(Sequence* node, std::ostream& ooo)
{
    ooo << "(basic-sequence";
    ++indent;
    space(ooo);
    ooo << ":items (";
    ++indent;
    for (auto ei : node->items) {
        space(ooo);
        convertAstNode(ei, ooo);
        ooo << " ";
    }
    --indent;
    --indent;
    space(ooo);
    ooo << "))";
    return 0;
}

///
int Lisper::convertSubroutine(Subroutine* node, std::ostream& ooo)
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
    convertAstNode(node->body, ooo);
    --indent;
    space(ooo);
    ooo << ")";
    --indent;
    return 0;
}

///
int Lisper::convertProgram(Program* node, std::ostream& ooo)
{
    ooo << "(basic-program :filename " << node->filename;
    ++indent;
    space(ooo);
    ooo << ":members (";
    for (auto si : node->members) {
        space(ooo);
        convertAstNode(si, ooo);
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
