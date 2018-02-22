
#ifndef ASDOT_HXX
#define ASDOT_HXX

#include "ast.hxx"
#include "converter.hxx"

#include <ostream>

namespace basic {
class Doter : public Converter {
public:
    Doter(std::ostream& os)
        : ooo(os)
    {}

private:
    int convertProgram(Program* node) override;
    int convertSubroutine(Subroutine* node) override;

    int convertSequence(Sequence* node) override;
    int convertLet(Let* node) override;
    int convertInput(Input* node) override;
    int convertPrint(Print* node) override;
    int convertIf(If* node) override;
    int convertWhile(While* node) override;
    int convertFor(For* node) override;
    int convertCall(Call* node) override;

    int convertApply(Apply* node) override;
    int convertBinary(Binary* node) override;
    int convertUnary(Unary* node) override;
    int convertVariable(Variable* node) override;
    int convertText(Text* node) override;
    int convertNumber(Number* node) override;

private:
    std::ostream& ooo;
    int index = 0;

    void connectNodes(int from, int to);
    void labeledNode(int num, const std::string& label);
};
} // basic

#endif // ASDOT_HXX
