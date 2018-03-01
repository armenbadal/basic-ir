
#ifndef Compiler_HXX
#define Compiler_HXX

#include "ast.hxx"

namespace basic {
class Compiler {
public:
    virtual ~Compiler() = default;
    virtual bool compile(AstNode* node);

protected:
    virtual void compileAstNode(AstNode* node);

    virtual void compileProgram(Program* node) = 0;
    virtual void compileSubroutine(Subroutine* node) = 0;

    virtual void compileSequence(Sequence* node) = 0;
    virtual void compileLet(Let* node) = 0;
    virtual void compileInput(Input* node) = 0;
    virtual void compilePrvoid(Prvoid* node) = 0;
    virtual void compileIf(If* node) = 0;
    virtual void compileWhile(While* node) = 0;
    virtual void compileFor(For* node) = 0;
    virtual void compileCall(Call* node) = 0;

    virtual void compileApply(Apply* node) = 0;
    virtual void compileBinary(Binary* node) = 0;
    virtual void compileUnary(Unary* node) = 0;
    virtual void compileVariable(Variable* node) = 0;
    virtual void compileText(Text* node) = 0;
    virtual void compileNumber(Number* node) = 0;
};
} // basic

#endif // Compiler_HXX
