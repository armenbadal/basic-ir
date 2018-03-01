
#ifndef ASLISP_HXX
#define ASLISP_HXX

#include "ast.hxx"
#include "compiler.hxx"

#include <ostream>

namespace basic {
class Lisper : public Compiler {
public:
    Lisper(std::ostream& os)
        : ooo(os)
    {}

private:
    void compileProgram(Program* node) override;
    void compileSubroutine(Subroutine* node) override;

    void compileSequence(Sequence* node) override;
    void compileLet(Let* node) override;
    void compileInput(Input* node) override;
    void compilePrint(Print* node) override;
    void compileIf(If* node) override;
    void compileWhile(While* node) override;
    void compileFor(For* node) override;
    void compileCall(Call* node) override;

    void compileApply(Apply* node) override;
    void compileBinary(Binary* node) override;
    void compileUnary(Unary* node) override;
    void compileVariable(Variable* node) override;
    void compileText(Text* node) override;
    void compileNumber(Number* node) override;

    void compileAstNode(AstNode* node) override;

private:
    std::ostream& ooo;
    int indent = 0;

    void space();
};
} // basic

#endif // ASLISP_HXX
