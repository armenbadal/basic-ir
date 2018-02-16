
#include "asdot.hxx"

namespace basic {
//
int convertProgram(Program* node, std::ostream& ooo)
{
    ooo << "\ndigraph G {\n";
    ooo << "\tprogram_0[label=\"PROGRAM\"];\n";
    for (Subroutine* si : node->members) {
        int sn = convertSubroutine(si, ooo);
        ooo << "program_0 -> subroutine_" << sn << ";\n";
    }
    ooo << "}\n\n";
    return 0;
}

//
int convertSubroutine(Subroutine* node, std::ostream& ooo)
{
    int cnum = index++;
    ooo << "\nsubroutine_" << cnum << "[shape=record,";
    ooo << "label=\"{SUBROUTINE|" << node->name << "}\"]";
    return cnum;
}

//
int convertSequence(Sequence* node, std::ostream& ooo)
{
    return 0;
}

//
int convertLet(Let* node, std::ostream& ooo)
{
    return 0;
}

//
int convertInput(Input* node, std::ostream& ooo)
{
    return 0;
}

//
int convertPrint(Print* node, std::ostream& ooo)
{
    return 0;
}

//
int convertIf(If* node, std::ostream& ooo)
{
    return 0;
}

//
int convertWhile(While* node, std::ostream& ooo)
{
    return 0;
}

//
int convertFor(For* node, std::ostream& ooo)
{
    return 0;
}

//
int convertCall(Call* node, std::ostream& ooo)
{
    return 0;
}

//
int convertApply(Apply* node, std::ostream& ooo)
{
    return 0;
}

//
int convertBinary(Binary* node, std::ostream& ooo)
{
    return 0;
}

//
int convertUnary(Unary* node, std::ostream& ooo)
{
    return 0;
}

//
int convertVariable(Variable* node, std::ostream& ooo)
{
    return 0;
}

//
int convertText(Text* node, std::ostream& ooo)
{
    return 0;
}

//
int convertNumber(Number* node, std::ostream& ooo)
{
    return 0;
}
} // basic
