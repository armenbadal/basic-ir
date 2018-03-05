
#ifndef AST_VISITOR_HXX
#define AST_VISITOR_HXX

#include "ast.hxx"

namespace basic {
    class AstVisitor {
    public:
        virtual ~AstVisitor() = default;

    protected:
        virtual void visitProgram(Program* node) = 0;
        virtual void visitSubroutine(Subroutine* node) = 0;

        virtual void visitSequence(Sequence* node) = 0;
        virtual void visitLet(Let* node) = 0;
        virtual void visitInput(Input* node) = 0;
        virtual void visitPrint(Print* node) = 0;
        virtual void visitIf(If* node) = 0;
        virtual void visitWhile(While* node) = 0;
        virtual void visitFor(For* node) = 0;
        virtual void visitCall(Call* node) = 0;

        virtual void visitApply(Apply* node) = 0;
        virtual void visitBinary(Binary* node) = 0;
        virtual void visitUnary(Unary* node) = 0;
        virtual void visitVariable(Variable* node) = 0;
        virtual void visitText(Text* node) = 0;
        virtual void visitNumber(Number* node) = 0;

        virtual void visitAstNode(AstNode* node);
    };
}

#endif // AST_VISITOR_HXX
