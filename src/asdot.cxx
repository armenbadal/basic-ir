
#include "asdot.hxx"

namespace basic {
//
int Doter::convertProgram(Program* node, std::ostream& ooo)
{
    ooo << "\ndigraph G {\n";
    ooo << "\tastnode_0[label=\"PROGRAM\"];\n";
    for (Subroutine* si : node->members) {
        int sn = convertSubroutine(si, ooo);
        ooo << "astnode_0 -> astnode_" << sn << ";\n";
    }
    ooo << "}\n\n";
    return 0;
}

//
int Doter::convertSubroutine(Subroutine* node, std::ostream& ooo)
{
    int cnum = index++;
    ooo << "\nastnode_" << cnum << "[shape=record,";
    ooo << "label=\"{SUBROUTINE|" << node->name << "}\"];";
	int bnum = convertAstNode(node->body, ooo);
	ooo << "astnode_" << cnum << " -> astnode_" << bnum << ";\n";
    return cnum;
}

//
int Doter::convertSequence(Sequence* node, std::ostream& ooo)
{
  int cnum = index++;
  ooo << "astnode_" << cnum << "[label=\"SEQUENCE\"];\n";
  for(AstNode* ni : node->items) {
	int ix = convertAstNode(ni, ooo);
	ooo << "astnode_" << cnum << " -> astnode_" << ix << ";\n";
  }
  return cnum;
}

//
int Doter::convertLet(Let* node, std::ostream& ooo)
{
    return 0;
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
    return 0;
}

//
int Doter::convertText(Text* node, std::ostream& ooo)
{
    return 0;
}

//
int Doter::convertNumber(Number* node, std::ostream& ooo)
{
    return 0;
}
} // basic
