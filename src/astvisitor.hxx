
#ifndef AST_VISITOR_HXX
#define AST_VISITOR_HXX

#include "ast.hxx"

#include <memory>

namespace basic {
class AstVisitor {
public:
    virtual ~AstVisitor() = default;
    
protected:
    virtual void visit(ProgramPtr node) = 0;
    virtual void visit(SubroutinePtr node) = 0;
    
    virtual void visit(SequencePtr node) = 0;
    virtual void visit(LetPtr node) = 0;
    virtual void visit(InputPtr node) = 0;
    virtual void visit(PrintPtr node) = 0;
    virtual void visit(IfPtr node) = 0;
    virtual void visit(WhilePtr node) = 0;
    virtual void visit(ForPtr node) = 0;
    virtual void visit(CallPtr node) = 0;

    virtual void visit(ApplyPtr node) = 0;
    virtual void visit(BinaryPtr node) = 0;
    virtual void visit(UnaryPtr node) = 0;
    virtual void visit(VariablePtr node) = 0;
    virtual void visit(TextPtr node) = 0;
    virtual void visit(NumberPtr node) = 0;
    virtual void visit(BooleanPtr node) = 0;
    
    virtual void visit( AstNodePtr node );
};
} // basic

#endif // AST_VISITOR_HXX
