#pragma once

#include "ast.hxx"
#include "astvisitor.hxx"
#include "diagnostics.hxx"
#include "symbol.hxx"
#include "types.hxx"

#include <optional>
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
    SemanticAnalyzer(
        SymbolTable& symbols,
        SemanticModel& semantic,
        Diagnostics& diagnostics);

    void analyze(const Program::Ptr& program);

private:
};

} // namespace basic
