#pragma once

#include "ast.hxx"
#include "astvisitor.hxx"

#include <filesystem>
#include <string>

namespace basic {
class Lisper : public ASTVisitorBase {
public:
    bool emitLisp(Program::Ptr node, const std::filesystem::path& file);

private:
    using ASTVisitorBase::visit;

    void visit(Program::Ptr node) override;
    void visit(Subroutine::Ptr node) override;

    void visit(Sequence::Ptr node) override;
    void visit(Dim::Ptr node) override;
    void visit(Let::Ptr node) override;
    void visit(Input::Ptr node) override;
    void visit(Print::Ptr node) override;
    void visit(If::Ptr node) override;
    void visit(While::Ptr node) override;
    void visit(For::Ptr node) override;
    void visit(Call::Ptr node) override;

    void visit(Apply::Ptr node) override;
    void visit(Binary::Ptr node) override;
    void visit(Unary::Ptr node) override;
    void visit(Variable::Ptr node) override;
    void visit(Text::Ptr node) override;
    void visit(Number::Ptr node) override;
    void visit(Boolean::Ptr node) override;

private:
    unsigned int indent{0};
    std::string _result;
};
} // basic
