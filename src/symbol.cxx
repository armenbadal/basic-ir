#include "symbol.hxx"

#include <unordered_map>

namespace basic {

// Տեսանելիության մեկ տիրույթը
class SymbolTable::Scope {
public:
    Scope() = default;

    // սահմանել/ավելացնել
    bool declare(std::string name, SymbolId id)
    {
        return _symbols.emplace(std::move(name), id).second;
    }

    // փնտրել
    std::optional<SymbolId> lookup(std::string_view name) const noexcept
    {
        auto it = _symbols.find(std::string{name});
        if( it == _symbols.end() )
            return std::nullopt;

        return it->second;
    }

private:
    // սիմվոլների համար "անուն" -> "ID" կապը
    std::unordered_map<std::string, SymbolId> _symbols;
};

SymbolTable::SymbolTable()
{
    openScope();
}

void SymbolTable::openScope()
{
    _scopes.emplace_back();
}

void SymbolTable::closeScope()
{
    if( _scopes.size() > 1 )
        _scopes.pop_back();
}


SymbolId SymbolTable::nextId()
{
    return _nextId++;
}

SymbolId SymbolTable::declareVariable(std::string name, Type::Ptr type, VariableSymbol::Storage storage)
{
    if( _scopes.back().lookup(name) )
        return 0;

    SymbolId id = nextId();
    _symbols.push_back(std::make_unique<VariableSymbol>(id, name, std::move(type), storage));
    _scopes.back().declare(std::move(name), id);
    return id;
}

SymbolId SymbolTable::declareParameter(std::string name, Type::Ptr type)
{
    if( _scopes.back().lookup(name) )
        return 0;

    SymbolId id = nextId();
    _symbols.push_back(std::make_unique<VariableSymbol>(id, name, std::move(type), VariableSymbol::Storage::Parameter));
    _scopes.back().declare(std::move(name), id);
    return id;
}

SymbolId SymbolTable::declareSubroutine(std::string name, std::vector<Type::Ptr> parameterTypes, std::optional<Type::Ptr> returnType)
{
    if( _scopes.back().lookup(name) )
        return 0;

    SymbolId id = nextId();
    _symbols.push_back(std::make_unique<SubroutineSymbol>(id, name, std::move(parameterTypes), std::move(returnType)));
    _scopes.back().declare(std::move(name), id);
    return id;
}

std::optional<SymbolId> SymbolTable::lookup(std::string_view name) const
{
    for( auto it = _scopes.rbegin(); it != _scopes.rend(); ++it )
        if( auto id = it->lookup(name) )
            return id;

    return std::nullopt;
}

std::optional<SymbolId> SymbolTable::lookupSubroutine(std::string_view name) const
{
    for( auto it = _scopes.rbegin(); it != _scopes.rend(); ++it )
        if( auto id = it->lookup(name) )
            if( symbol(*id).kind() == Symbol::Kind::Subroutine )
                return id;

    return std::nullopt;
}

bool SymbolTable::exists(std::string_view name) const
{
    return lookup(name).has_value();
}

Symbol& SymbolTable::symbol(SymbolId id)
{
    return *_symbols.at(id - 1);
}

const Symbol& SymbolTable::symbol(SymbolId id) const
{
    return *_symbols.at(id - 1);
}

} // namespace basic
