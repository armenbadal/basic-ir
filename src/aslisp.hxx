
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
    void visitProgram(Program* node);
    void visitSubroutine(Subroutine* node);

    void visitSequence(Sequence* node);
    void visitLet(Let* node);
    void visitInput(Input* node);
    void visitPrint(Print* node);
    void visitIf(If* node);
    void visitWhile(While* node);
    void visitFor(For* node);
    void visitCall(Call* node);

    void visitApply(Apply* node);
    void visitBinary(Binary* node);
    void visitUnary(Unary* node);
    void visitVariable(Variable* node);
    void visitText(Text* node);
    void visitNumber(Number* node);

    void visitAstNode(AstNode* node);

private:
    std::ostream& ooo;
    int indent = 0;
};
} // basic

#endif // ASLISP_HXX
