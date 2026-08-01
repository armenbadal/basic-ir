#pragma once

#include "ast.hxx"
#include "astvisitor.hxx"
#include "diagnostics.hxx"
#include "symbol.hxx"
#include "types.hxx"

#include <optional>

namespace basic {

class SemanticModel {
public:
    void bind(NodeId, SymbolId);
    void setType(NodeId, Type);
    std::optional<SymbolId> symbol(NodeId) const;
    std::optional<Type> type(NodeId) const;

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

}// namespace basic 
