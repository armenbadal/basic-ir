#pragma once

#include "ast.hxx"
#include "astvisitor.hxx"
#include "diagnostics.hxx"
#include "symbol.hxx"
#include "types.hxx"

#include <optional>
#include <string_view>
#include <unordered_map>

namespace basic {

class SemanticModel {
public:
    void bind(NodeId node, SymbolId symbol);
    void setType(NodeId node, Type::Ptr type);
    std::optional<SymbolId> symbol(NodeId node) const;
    std::optional<Type::Ptr> type(NodeId node) const;

private:
    std::unordered_map<NodeId, SymbolId> _symbols;
    std::unordered_map<NodeId, Type::Ptr> _types;
};

class SemanticAnalyzer : public ASTVisitor<SemanticAnalyzer> {
public:
    SemanticAnalyzer(SymbolTable& symbols, SemanticModel& semantic, Diagnostics& diagnostics);

    void analyze(const Program::Ptr& program);

    using ASTVisitor<SemanticAnalyzer>::visit;

    void visit(Program& node);
    void visit(Subroutine& node);
    void visit(Sequence& node);
    void visit(Dim& node);
    void visit(Let& node);
    void visit(Input& node);
    void visit(Print& node);
    void visit(If& node);
    void visit(If::IfThen& node);
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

private:
    Type::Ptr expressionType(const Expression::Ptr& expression);
    std::optional<SymbolId> variableSymbol(const Variable& variable);
    void requireType(const Node& node, const Type& actual, const Type& expected, std::string_view context);

    void error(const Node& node, std::string_view fmt, auto&&... args)
    {
        _diagnostics.advance();
        const auto msg = std::vformat(fmt, std::make_format_args(args...));
        _diagnostics.mark(node.line, msg);
    }

    SymbolTable& _symbols;
    SemanticModel& _semantic;
    Diagnostics& _diagnostics;
};

} // namespace basic
