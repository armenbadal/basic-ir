
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

    bool asLisp( AstNodePtr node );

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

private:
    std::ostream& ooo;
    int indent = 0;
};
} // basic

#endif // ASLISP_HXX
