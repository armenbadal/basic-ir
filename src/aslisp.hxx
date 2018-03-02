
#ifndef ASLISP_HXX
#define ASLISP_HXX

#include "ast.hxx"

#include <ostream>

namespace basic {
class Lisper {
public:
    Lisper(std::ostream& os)
        : ooo(os)
    {}

    bool asLisp(AstNode* node);

private:
    void asLispProgram(Program* node);
    void asLispSubroutine(Subroutine* node);

    void asLispSequence(Sequence* node);
    void asLispLet(Let* node);
    void asLispInput(Input* node);
    void asLispPrint(Print* node);
    void asLispIf(If* node);
    void asLispWhile(While* node);
    void asLispFor(For* node);
    void asLispCall(Call* node);

    void asLispApply(Apply* node);
    void asLispBinary(Binary* node);
    void asLispUnary(Unary* node);
    void asLispVariable(Variable* node);
    void asLispText(Text* node);
    void asLispNumber(Number* node);

    void asLispAstNode(AstNode* node);

private:
    std::ostream& ooo;
    int indent = 0;
};
} // basic

#endif // ASLISP_HXX
