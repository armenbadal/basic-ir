
#ifndef CHECKER_HXX
#define CHECKER_HXX

#include "ast.hxx"
#include "astvisitor.hxx"

#include <memory>
#include <optional>
#include <ostream>

namespace basic {
class Checker : public AstVisitor {
public:
    std::optional<std::string> check(AstNodePtr node);

private:
    void visit(ProgramPtr node) override;
    void visit(SubroutinePtr node) override;

    void visit(SequencePtr node) override;
    void visit(LetPtr node) override;
    void visit(InputPtr node) override;
    void visit(PrintPtr node) override;
    void visit(IfPtr node) override;
    void visit(WhilePtr node) override;
    void visit(ForPtr node) override;
    void visit(CallPtr node) override;

    void visit(ApplyPtr node) override;
    void visit(BinaryPtr node) override;
    void visit(UnaryPtr node) override;
    void visit(VariablePtr node) override;
    void visit(TextPtr node) override;
    void visit(NumberPtr node) override;
    void visit(BooleanPtr node) override;

    void visit(AstNodePtr node) override;
};
} // basic

#endif // CHECKER_HXX


