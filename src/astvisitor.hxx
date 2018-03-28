
#ifndef AST_VISITOR_HXX
#define AST_VISITOR_HXX

#include "ast.hxx"

#include <memory>

namespace basic {
class AstVisitor {
public:
    virtual ~AstVisitor() = default;
    
protected:
    virtual void visitProgram( ProgramPtr node ) = 0;
    virtual void visitSubroutine( SubroutinePtr node ) = 0;
    
    virtual void visitSequence( SequencePtr node ) = 0;
    virtual void visitLet( LetPtr node ) = 0;
    virtual void visitInput( InputPtr node ) = 0;
    virtual void visitPrint( PrintPtr node ) = 0;
    virtual void visitIf( IfPtr node ) = 0;
    virtual void visitWhile( WhilePtr node ) = 0;
    virtual void visitFor( ForPtr node ) = 0;
    virtual void visitCall( CallPtr node ) = 0;

    virtual void visitApply( ApplyPtr node ) = 0;
    virtual void visitBinary( BinaryPtr node ) = 0;
    virtual void visitUnary( UnaryPtr node ) = 0;
    virtual void visitVariable( VariablePtr node ) = 0;
    virtual void visitText( TextPtr node ) = 0;
    virtual void visitNumber( NumberPtr node ) = 0;
    
    virtual void visitAstNode( AstNodePtr node );
};
} // basic

#endif // AST_VISITOR_HXX
