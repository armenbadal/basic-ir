
#ifndef ASLISP_HXX
#define ASLISP_HXX

#include "ast.hxx"
#include "astvisitor.hxx"

#include <filesystem>
#include <fstream>


namespace basic {
class Lisper : public AstVisitor {
public:
    bool emitLisp(AstNodePtr node, const std::filesystem::path& file);

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

    void visit(AstNodePtr node) override;
    
private:
    std::ofstream os;
    unsigned int indent{0};
};
} // basic

#endif // ASLISP_HXX
