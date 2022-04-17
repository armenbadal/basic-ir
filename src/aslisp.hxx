
#ifndef ASLISP_HXX
#define ASLISP_HXX

#include "ast.hxx"
#include "astvisitor.hxx"

#include <filesystem>
#include <string>

namespace basic {
class Lisper : public ASTVisitor<std::string,std::string,std::string,std::string> {
public:
    bool emitLisp(ProgramPtr node, const std::filesystem::path& file);

private:
    std::string visit(ProgramPtr node) override;
    std::string visit(SubroutinePtr node) override;

    std::string visit(StatementPtr node) override;
    std::string visit(SequencePtr node) override;
    std::string visit(LetPtr node) override;
    std::string visit(InputPtr node) override;
    std::string visit(PrintPtr node) override;
    std::string visit(IfPtr node) override;
    std::string visit(WhilePtr node) override;
    std::string visit(ForPtr node) override;
    std::string visit(CallPtr node) override;

    std::string visit(ExpressionPtr node) override;
    std::string visit(ApplyPtr node) override;
    std::string visit(BinaryPtr node) override;
    std::string visit(UnaryPtr node) override;
    std::string visit(VariablePtr node) override;
    std::string visit(TextPtr node) override;
    std::string visit(NumberPtr node) override;
    std::string visit(BooleanPtr node) override;

private:
    unsigned int indent{0};
};
} // basic

#endif // ASLISP_HXX
