#pragma once

#include "ast.hxx"
#include "astvisitor.hxx"

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace basic {
class Checker : public ASTVisitorBase {
public:
    std::optional<std::string> check(Program::Ptr node);

private:
    using ASTVisitorBase::visit;

    void visit(Program::Ptr node) override;
    void visit(Subroutine::Ptr node) override;

    void visit(Sequence::Ptr node) override;
    void visit(Dim::Ptr node) override;
    void visit(Let::Ptr node) override;
    void visit(If::Ptr node) override;
    void visit(While::Ptr node) override;
    void visit(For::Ptr node) override;
    void visit(Call::Ptr node) override;

    void visit(Array::Ptr node) override;
    void visit(Apply::Ptr node) override;
    void visit(Binary::Ptr node) override;
    void visit(Unary::Ptr node) override;
    void visit(Variable::Ptr node) override;
    void visit(Text::Ptr node) override;
    void visit(Number::Ptr node) override;
    void visit(Boolean::Ptr node) override;

    std::unordered_map<std::string_view, Subroutine::Ptr> subroutines;
};
} // basic
