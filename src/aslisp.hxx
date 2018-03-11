
#ifndef ASLISP_HXX
#define ASLISP_HXX

#include "ast.hxx"
#include "astvisitor.hxx"

#include <ostream>

namespace basic {
class Lisper : public AstVisitor {
public:
    Lisper(std::ostream& os)
        : ooo(os)
    {}
    ~Lisper() = default;

    bool asLisp( std::shared_ptr<AstNode> node );

private:
    void visitProgram( std::shared_ptr<Program> node ) override;
    void visitSubroutine( std::shared_ptr<Subroutine> node ) override;

    void visitSequence( std::shared_ptr<Sequence> node ) override;
    void visitLet( std::shared_ptr<Let> node ) override;
    void visitInput( std::shared_ptr<Input> node ) override;
    void visitPrint( std::shared_ptr<Print> node ) override;
    void visitIf( std::shared_ptr<If> node ) override;
    void visitWhile( std::shared_ptr<While> node ) override;
    void visitFor( std::shared_ptr<For> node ) override;
    void visitCall( std::shared_ptr<Call> node ) override;

    void visitApply( std::shared_ptr<Apply> node ) override;
    void visitBinary( std::shared_ptr<Binary> node ) override;
    void visitUnary( std::shared_ptr<Unary> node ) override;
    void visitVariable( std::shared_ptr<Variable> node ) override;
    void visitText( std::shared_ptr<Text> node ) override;
    void visitNumber( std::shared_ptr<Number> node ) override;

    void visitAstNode( std::shared_ptr<AstNode> node ) override;

private:
    std::ostream& ooo;
    int indent = 0;
};
} // basic

#endif // ASLISP_HXX
