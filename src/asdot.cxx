
#include "asdot.hxx"

namespace basic {
//
int Doter::convertProgram(Program* node)
{
    int cnum = index++;
    ooo << "\ndigraph G {\n";
    ooo << "\tastnode_" << cnum << "[label=\"PROGRAM\"];\n";
    for (Subroutine* si : node->members) {
        int sn = convertSubroutine(si);
        connectNodes(cnum, sn);
    }
    ooo << "}\n\n";
    return cnum;
}

//
int Doter::convertSubroutine(Subroutine* node)
{
    int cnum = index++;
    ooo << "\tastnode_" << cnum << "[shape=record,";
    ooo << "label=\"{SUBROUTINE|" << node->name << "}\"];\n";
    int bnum = convertAstNode(node->body);
    connectNodes(cnum, bnum);
    return cnum;
}

//
int Doter::convertSequence(Sequence* node)
{
    int cnum = index++;
    ooo << "\tastnode_" << cnum << "[label=\"SEQUENCE\"];\n";
    for (AstNode* ni : node->items) {
        int ix = convertAstNode(ni);
        connectNodes(cnum, ix);
    }
    return cnum;
}

//
int Doter::convertLet(Let* node)
{
    int cnum = index++;
    ooo << "\tastnode_" << cnum << "[label=\"LET\"];\n";
    int il = convertVariable(node->varptr);
    connectNodes(cnum, il);
    int ir = convertAstNode(node->expr);
    connectNodes(cnum, ir);
    return cnum;
}

//
int Doter::convertInput(Input* node)
{
    return 0;
}

//
int Doter::convertPrint(Print* node)
{
    return 0;
}

//
int Doter::convertIf(If* node)
{
    return 0;
}

//
int Doter::convertWhile(While* node)
{
    return 0;
}

//
int Doter::convertFor(For* node)
{
    return 0;
}

//
int Doter::convertCall(Call* node)
{
    return 0;
}

//
int Doter::convertApply(Apply* node)
{
    int cnum = index++;
    ooo << "\tastnode_" << cnum << "[label=\"" << "APPLY" << "\"];\n";
    int pnum = index++;
    ooo << "\tastnode_" << pnum << "[label=\"" << node->procptr->name << "\"];\n";
    connectNodes(cnum, pnum);
    //
    return cnum;
}

//
int Doter::convertBinary(Binary* node)
{
    int cnum = index++;
    ooo << "\tastnode_" << cnum << "[label=\"" << operationName(node->opcode) << "\"];\n";
    int il = convertAstNode(node->subexpro);
    connectNodes(cnum, il);
    int ir = convertAstNode(node->subexpri);
    connectNodes(cnum, ir);
    return cnum;
}

//
int Doter::convertUnary(Unary* node)
{
    int cnum = index++;
    ooo << "\tastnode_" << cnum << "[label=\"" << operationName(node->opcode) << "\"];\n";
    int ic = convertAstNode(node->subexpr);
    connectNodes(cnum, ic);
    return cnum;
}

//
int Doter::convertVariable(Variable* node)
{
    int cnum = index++;
    ooo << "\tastnode_" << cnum << "[label=\"VARIABLE: " << node->name << "\"];\n";
    return cnum;
}

//
int Doter::convertText(Text* node)
{
    int cnum = index++;
    ooo << "\tastnode_" << cnum << "[label=\"TEXT: " << node->value << "\"];\n";
    return cnum;
}

//
int Doter::convertNumber(Number* node)
{
    int cnum = index++;
    ooo << "\tastnode_" << cnum << "[label=\"NUMBER: " << node->value << "\"];\n";
    return cnum;
}

//
void Doter::connectNodes(int from, int to)
{
    ooo << "\tastnode_" << from << " -> " << "astnode_" << to << ";\n";
}
} // basic
