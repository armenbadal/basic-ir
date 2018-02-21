
#ifndef ASLISP_HXX
#define ASLISP_HXX

#include "ast.hxx"
#include "converter.hxx"

#include <ostream>

namespace basic {
class Lisper : public Converter {
public:
    Lisper(std::ostream& os)
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
    int indent = 0;

    void space(std::ostream& ooo);
};
} // basic

#endif // ASLISP_HXX
