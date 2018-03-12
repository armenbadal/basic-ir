
#ifndef AST_VISITOR_HXX
#define AST_VISITOR_HXX

#include "ast.hxx"

#include <memory>

namespace basic {
    class AstVisitor {
    public:
        virtual ~AstVisitor() = default;

    protected:
        virtual void visitProgram( std::shared_ptr<Program> node ) = 0;
        virtual void visitSubroutine( std::shared_ptr<Subroutine> node ) = 0;

        virtual void visitSequence( std::shared_ptr<Sequence> node ) = 0;
        virtual void visitLet( std::shared_ptr<Let> node ) = 0;
        virtual void visitInput( std::shared_ptr<Input> node ) = 0;
        virtual void visitPrint( std::shared_ptr<Print> node ) = 0;
        virtual void visitIf( std::shared_ptr<If> node ) = 0;
        virtual void visitWhile( std::shared_ptr<While> node ) = 0;
        virtual void visitFor( std::shared_ptr<For> node ) = 0;
        virtual void visitCall( std::shared_ptr<Call> node ) = 0;

        virtual void visitApply( std::shared_ptr<Apply> node ) = 0;
        virtual void visitBinary( std::shared_ptr<Binary> node ) = 0;
        virtual void visitUnary( std::shared_ptr<Unary> node ) = 0;
        virtual void visitVariable( std::shared_ptr<Variable> node ) = 0;
        virtual void visitText( std::shared_ptr<Text> node ) = 0;
        virtual void visitNumber( std::shared_ptr<Number> node ) = 0;

        virtual void visitAstNode( std::shared_ptr<AstNode> node );
    };
}

#endif // AST_VISITOR_HXX
