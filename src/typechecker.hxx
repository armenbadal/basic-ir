
#ifndef TYPE_CHECKER_HXX
#define TYPE_CHECKER_HXX

#include "ast.hxx"
#include "astvisitor.hxx"

namespace basic {
    class TypeChecker : public AstVisitor {
    public:
        bool check(AstNode* node);

    private:
        void visitProgram(Program* node) override;
        void visitSubroutine(Subroutine* node) override;

        void visitSequence(Sequence* node) override;
        void visitLet(Let* node) override;
        void visitInput(Input* node) override;
        void visitPrint(Print* node) override;
        void visitIf(If* node) override;
        void visitWhile(While* node) override;
        void visitFor(For* node) override;
        void visitCall(Call* node) override;

        void visitApply(Apply* node) override;
        void visitBinary(Binary* node) override;
        void visitUnary(Unary* node) override;
        void visitVariable(Variable* node) override;
        void visitText(Text* node) override;
        void visitNumber(Number* node) override;
    };
}

#endif // TYPE_CHECKER_HXX
