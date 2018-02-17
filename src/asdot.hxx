
#ifndef ASDOT_HXX
#define ASDOT_HXX

#include "ast.hxx"
#include "converter.hxx"

namespace basic {
class Doter : public Converter {
private:
    int convertProgram(Program* node, std::ostream& ooo) override;
    int convertSubroutine(Subroutine* node, std::ostream& ooo) override;

    int convertSequence(Sequence* node, std::ostream& ooo) override;
    int convertLet(Let* node, std::ostream& ooo) override;
    int convertInput(Input* node, std::ostream& ooo) override;
    int convertPrint(Print* node, std::ostream& ooo) override;
    int convertIf(If* node, std::ostream& ooo) override;
    int convertWhile(While* node, std::ostream& ooo) override;
    int convertFor(For* node, std::ostream& ooo) override;
    int convertCall(Call* node, std::ostream& ooo) override;

    int convertApply(Apply* node, std::ostream& ooo) override;
    int convertBinary(Binary* node, std::ostream& ooo) override;
    int convertUnary(Unary* node, std::ostream& ooo) override;
    int convertVariable(Variable* node, std::ostream& ooo) override;
    int convertText(Text* node, std::ostream& ooo) override;
    int convertNumber(Number* node, std::ostream& ooo) override;

private:
    int index = 0;
};
} // basic

#endif // ASDOT_HXX
