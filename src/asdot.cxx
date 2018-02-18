
#include "asdot.hxx"

namespace basic {
//
int Doter::convertProgram(Program* node, std::ostream& ooo)
{
    int cnum = index++;
    ooo << "\ndigraph G {\n";
    ooo << "\tastnode_" << cnum << "[label=\"PROGRAM\"];\n";
    for (Subroutine* si : node->members) {
        int sn = convertSubroutine(si, ooo);
        ooo << "\tastnode_" << cnum << " -> astnode_" << sn << ";\n";
    }
    ooo << "}\n\n";
    return cnum;
}

//
int Doter::convertSubroutine(Subroutine* node, std::ostream& ooo)
{
    int cnum = index++;
    ooo << "\tastnode_" << cnum << "[shape=record,";
    ooo << "label=\"{SUBROUTINE|" << node->name << "}\"];\n";
    int bnum = convertAstNode(node->body, ooo);
    ooo << "\tastnode_" << cnum << " -> astnode_" << bnum << ";\n";
    return cnum;
}

//
int Doter::convertSequence(Sequence* node, std::ostream& ooo)
{
    int cnum = index++;
    ooo << "\tastnode_" << cnum << "[label=\"SEQUENCE\"];\n";
    for (AstNode* ni : node->items) {
        int ix = convertAstNode(ni, ooo);
        ooo << "\tastnode_" << cnum << " -> astnode_" << ix << ";\n";
    }
    return cnum;
}

//
int Doter::convertLet(Let* node, std::ostream& ooo)
{
    int cnum = index++;
    ooo << "\tastnode_" << cnum << "[label=\"LET\"];\n";
    int il = convertVariable(node->varptr, ooo);
    ooo << "\tastnode_" << cnum << " -> "
        << "astnode_" << il << ";\n";
    int ir = convertAstNode(node->expr, ooo);
    ooo << "\tastnode_" << cnum << " -> "
        << "astnode_" << ir << ";\n";
    return cnum;
}

//
int Doter::convertInput(Input* node, std::ostream& ooo)
{
    return 0;
}

//
int Doter::convertPrint(Print* node, std::ostream& ooo)
{
    return 0;
}

//
int Doter::convertIf(If* node, std::ostream& ooo)
{
    return 0;
}

//
int Doter::convertWhile(While* node, std::ostream& ooo)
{
    return 0;
}

//
int Doter::convertFor(For* node, std::ostream& ooo)
{
    return 0;
}

//
int Doter::convertCall(Call* node, std::ostream& ooo)
{
    return 0;
}

//
int Doter::convertApply(Apply* node, std::ostream& ooo)
{
    return 0;
}

//
int Doter::convertBinary(Binary* node, std::ostream& ooo)
{
    return 0;
}

//
int Doter::convertUnary(Unary* node, std::ostream& ooo)
{
    return 0;
}

//
int Doter::convertVariable(Variable* node, std::ostream& ooo)
{
    int cnum = index++;
    ooo << "\tastnode_" << cnum << "[label=\"VARIABLE: " << node->name << "\"];\n";
    return cnum;
}

//
int Doter::convertText(Text* node, std::ostream& ooo)
{
    return 0;
}

//
int Doter::convertNumber(Number* node, std::ostream& ooo)
{
    int cnum = index++;
    ooo << "\tastnode_" << cnum << "[label=\"NUMBER: " << node->value << "\"];\n";
    return cnum;
}
} // basic
