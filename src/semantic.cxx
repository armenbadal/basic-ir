#include "semantic.hxx"

namespace basic {

void SemanticModel::bind(NodeId node, SymbolId symbol)
{
}

void SemanticModel::setType(NodeId node, Type::Ptr type)
{
}

std::optional<SymbolId> SemanticModel::symbol(NodeId node) const
{
    return std::nullopt;
}

std::optional<Type::Ptr> SemanticModel::type(NodeId node) const
{
    return std::nullopt;
}

}
