
#include "asdot.hxx"

namespace basic {
//
int Doter::convertProgram(Program* node)
{
    int cnum = index++;
    ooo << "\ndigraph G {\n";
    ooo << "\tastnode_" << cnum << "[label=\"PROGRAM\"];\n";
    ooo << "\tnode[shape=box];\n";
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
    ooo << "\n\tsubgraph cluster_" << node->name << "{\n";
    ooo << "\t\tastnode_" << cnum << "[shape=record,label=\"{SUBROUTINE|" << node->name << "}\"];\n";
    int bnum = convertAstNode(node->body);
    connectNodes(cnum, bnum);
    ooo << "\t}\n";
    return cnum;
}

//
int Doter::convertSequence(Sequence* node)
{
    int cnum = index++;
    ooo << "\t\tastnode_" << cnum << "[label=\"SEQUENCE\"];\n";
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
    labeledNode(cnum, "LET");
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
    int cnum = index++;
    labeledNode(cnum, "PRINT");
    int anum = convertAstNode(node->expr);
    connectNodes(cnum, anum);
    return cnum;
}

//
int Doter::convertIf(If* node)
{
    int cnum = index++;
    labeledNode(cnum, "IF");
    int e0 = convertAstNode(node->condition);
    int e1 = convertAstNode(node->decision);
    int e2 = convertAstNode(node->alternative);
    return cnum;
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
    labeledNode(cnum, "APPLY");
    int pnum = index++;
    labeledNode(pnum, node->procptr->name);
    connectNodes(cnum, pnum);
    for (auto ai : node->arguments) {
        int anum = convertAstNode(ai);
        connectNodes(pnum, anum);
    }
    return cnum;
}

//
int Doter::convertBinary(Binary* node)
{
    int cnum = index++;
    labeledNode(cnum, toString(node->opcode));
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
    labeledNode(cnum, toString(node->opcode));
    int ic = convertAstNode(node->subexpr);
    connectNodes(cnum, ic);
    return cnum;
}

//
int Doter::convertVariable(Variable* node)
{
    int cnum = index++;
    labeledNode(cnum, "VARIABLE: " + node->name);
    return cnum;
}

//
int Doter::convertText(Text* node)
{
    int cnum = index++;
    labeledNode(cnum, "TEXT: " + node->value);
    return cnum;
}

//
int Doter::convertNumber(Number* node)
{
    int cnum = index++;
    labeledNode(cnum, "NUMBER: " + std::to_string(node->value));
    return cnum;
}

//
void Doter::connectNodes(int from, int to)
{
    ooo << "\t\tastnode_" << from << " -> " << "astnode_" << to << ";\n";
}

//
void Doter::labeledNode(int num, const std::string& label)
{
    ooo << "\t\tastnode_" << num << "[label=\"" << label << "\"];\n";
}
} // basic
