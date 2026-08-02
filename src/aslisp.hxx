#pragma once

#include "ast.hxx"
#include "astvisitor.hxx"

#include <ostream>
#include <ranges>
#include <string>

namespace basic {
class Lisper : public ASTVisitor<Lisper, std::string> {
public:
    void emit(Program::Ptr node, std::ostream& os);

    using ASTVisitor<Lisper, std::string>::visit;

    std::string visit(Program& node);
    std::string visit(Subroutine& node);

    std::string visit(Sequence& node);
    std::string visit(Dim& node);
    std::string visit(Let& node);
    std::string visit(If& node);
    std::string visit(If::IfThen& node);
    std::string visit(While& node);
    std::string visit(For& node);
    std::string visit(Call& node);

    std::string visit(Array& node);
    std::string visit(Apply& node);
    std::string visit(Binary& node);
    std::string visit(Unary& node);
    std::string visit(Variable& node);
    std::string visit(Text& node);
    std::string visit(Number& node);
    std::string visit(Boolean& node);

    template<typename T>
    std::string visitVector(const std::vector<T>& elems)
    {
        return elems
            | std::views::transform([this](const auto& e) { return visit(*e); })
            | std::views::join_with(' ')
            | std::ranges::to<std::string>();
    }

    // Նույնը, բայց բացատով նախածանցված։ Դատարկ ցուցակի դեպքում ոչինչ չի
    // ավելացնում, որպեսզի փակող փակագծից առաջ ավելորդ բացատ չմնա
    template<typename T>
    std::string spaced(const std::vector<T>& elems)
    {
        auto joined = visitVector(elems);
        return joined.empty() ? joined : " " + joined;
    }
};
} // basic
