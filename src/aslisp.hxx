#pragma once

#include "ast.hxx"
#include "astvisitor.hxx"

#include <ostream>
#include <ranges>
#include <string>

namespace basic {
class Lisper : public ASTVisitor<Lisper> {
public:
    void emit(Program::Ptr node, std::ostream& os);

    using ASTVisitor<Lisper>::visit;

    void visit(Program& node);
    void visit(Subroutine& node);

    void visit(Sequence& node);
    void visit(Dim& node);
    void visit(Let& node);
    void visit(Input& node);
    void visit(Print& node);
    void visit(If& node);
    void visit(While& node);
    void visit(For& node);
    void visit(Call& node);

    void visit(Array& node);
    void visit(Apply& node);
    void visit(Binary& node);
    void visit(Unary& node);
    void visit(Variable& node);
    void visit(Text& node);
    void visit(Number& node);
    void visit(Boolean& node);

    template<typename T>
    std::string str(T node)
    {
        visit(*node);
        return _result;
    }

    template<typename T>
    std::string str(const std::vector<T>& elems)
    {
        return elems
            | std::views::transform([this](auto e) { return str(e); })
            | std::views::join_with(' ')
            | std::ranges::to<std::string>();
    }

private:
    std::string _result;
};
} // basic
