#include "semantic.hxx"

namespace basic {

void SemanticModel::bind(NodeId node, SymbolId symbol)
{
    _symbols.insert_or_assign(node, symbol);
}

void SemanticModel::setType(NodeId node, Type::Ptr type)
{
    _types.insert_or_assign(node, std::move(type));
}

std::optional<SymbolId> SemanticModel::symbol(NodeId node) const
{
    if (auto it = _symbols.find(node); it != _symbols.end())
        return it->second;

    return std::nullopt;
}

std::optional<Type::Ptr> SemanticModel::type(NodeId node) const
{
    if (auto it = _types.find(node); it != _types.end())
        return it->second;

    return std::nullopt;
}

}
