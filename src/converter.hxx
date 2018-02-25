
#ifndef CONVERTER_HXX
#define CONVERTER_HXX

#include "ast.hxx"

namespace basic {
class Converter {
public:
    virtual ~Converter() = default;
    virtual bool convert(AstNode* node);

protected:
    virtual int convertAstNode(AstNode* node);

    virtual int convertProgram(Program* node) = 0;
    virtual int convertSubroutine(Subroutine* node) = 0;

    virtual int convertSequence(Sequence* node) = 0;
    virtual int convertLet(Let* node) = 0;
    virtual int convertInput(Input* node) = 0;
    virtual int convertPrint(Print* node) = 0;
    virtual int convertIf(If* node) = 0;
    virtual int convertWhile(While* node) = 0;
    virtual int convertFor(For* node) = 0;
    virtual int convertCall(Call* node) = 0;

    virtual int convertApply(Apply* node) = 0;
    virtual int convertBinary(Binary* node) = 0;
    virtual int convertUnary(Unary* node) = 0;
    virtual int convertVariable(Variable* node) = 0;
    virtual int convertText(Text* node) = 0;
    virtual int convertNumber(Number* node) = 0;
};
} // basic

#endif // CONVERTER_HXX
