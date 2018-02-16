
#ifndef CONVERTER_HXX
#define CONVERTER_HXX

#include "ast.hxx"

#include <ostream>
#include <string>

namespace basic {
class Converter {
public:
    virtual ~Converter() = default;
    virtual bool convert(AstNode* node, std::ostream& ooo);

protected:
    virtual int convertAstNode(AstNode* node, std::ostream& ooo);

    virtual int convertProgram(Program* node, std::ostream& ooo) = 0;
    virtual int convertSubroutine(Subroutine* node, std::ostream& ooo) = 0;

    virtual int convertSequence(Sequence* node, std::ostream& ooo) = 0;
    virtual int convertLet(Let* node, std::ostream& ooo) = 0;
    virtual int convertInput(Input* node, std::ostream& ooo) = 0;
    virtual int convertPrint(Print* node, std::ostream& ooo) = 0;
    virtual int convertIf(If* node, std::ostream& ooo) = 0;
    virtual int convertWhile(While* node, std::ostream& ooo) = 0;
    virtual int convertFor(For* node, std::ostream& ooo) = 0;
    virtual int convertCall(Call* node, std::ostream& ooo) = 0;

    virtual int convertApply(Apply* node, std::ostream& ooo) = 0;
    virtual int convertBinary(Binary* node, std::ostream& ooo) = 0;
    virtual int convertUnary(Unary* node, std::ostream& ooo) = 0;
    virtual int convertVariable(Variable* node, std::ostream& ooo) = 0;
    virtual int convertText(Text* node, std::ostream& ooo) = 0;
    virtual int convertNumber(Number* node, std::ostream& ooo) = 0;
};
} // basic

#endif // CONVERTER_HXX

