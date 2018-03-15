
#ifndef TYPE_CHECKER_HXX
#define TYPE_CHECKER_HXX

#include "ast.hxx"
#include "astvisitor.hxx"

#include <memory>

namespace basic {
class TypeChecker : public AstVisitor {
public:
    bool check( AstNodePtr node);

private:
    void visitProgram( ProgramPtr node ) override;
    void visitSubroutine( SubroutinePtr node ) override;

    void visitSequence( SequencePtr node ) override;
    void visitLet( LetPtr node ) override;
    void visitInput( InputPtr node ) override;
    void visitPrint( PrintPtr node ) override;
    void visitIf( IfPtr node ) override;
    void visitWhile( WhilePtr node ) override;
    void visitFor( ForPtr node ) override;
    void visitCall( CallPtr node ) override;

    void visitApply( ApplyPtr node ) override;
    void visitBinary( BinaryPtr node ) override;
    void visitUnary( UnaryPtr node ) override;
    void visitVariable( VariablePtr node ) override;
    void visitText( TextPtr node ) override;
    void visitNumber( NumberPtr node ) override;

    void visitAstNode( AstNodePtr node ) override;
};
}

#endif // TYPE_CHECKER_HXX
