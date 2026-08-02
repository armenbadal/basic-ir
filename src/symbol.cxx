#include "symbol.hxx"

#include <unordered_map>

namespace basic {

Symbol::Symbol(SymbolId id, std::string name)
    : _name{std::move(name)}, _id{id}
{
}

SymbolId Symbol::id() const noexcept
{
    return _id;
}

std::string_view Symbol::name() const noexcept
{
    return _name;
}


VariableSymbol::VariableSymbol(SymbolId id, std::string name, Type::Ptr type)
    : Symbol{id, std::move(name)}
    , _type{std::move(type)}
{
}

const Type& VariableSymbol::type() const noexcept
{
    return *_type;
}

Symbol::Kind VariableSymbol::kind() const noexcept
{
    return Symbol::Kind::Variable;
}

SubroutineSymbol::SubroutineSymbol(SymbolId id, std::string name, std::vector<SymbolId> parameters)
    : Symbol{id, std::move(name)}
    , _parameters{std::move(parameters)}
{
}

const std::vector<SymbolId>& SubroutineSymbol::parameters() const
{
    return _parameters;
}

Symbol::Kind SubroutineSymbol::kind() const noexcept
{
    return Symbol::Kind::Subroutine;
}



class SymbolTable::Scope {
public:
    Scope() = default;

    bool declare(std::string name, SymbolId id)
    {
        return _symbols.emplace(std::move(name), id).second;
    }

    std::optional<SymbolId> lookup(std::string_view name) const noexcept
    {
        auto it = _symbols.find(std::string{name});

        if (it == _symbols.end())
            return std::nullopt;

        return it->second;
    }

private:
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

SymbolId SymbolTable::declareVariable(std::string name, Type::Ptr type)
{
    SymbolId id = nextId();
    _symbols.push_back(std::make_unique<VariableSymbol>(id, name, std::move(type)));
    _scopes.back().declare(std::move(name), id);
    return id;
}

SymbolId SymbolTable::declareParameter(std::string name, Type::Ptr type)
{
    return declareVariable(std::move(name), std::move(type));
}

SymbolId SymbolTable::declareSubroutine(std::string name, std::vector<SymbolId> parameters)
{
    SymbolId id = nextId();
    _symbols.push_back(std::make_unique<SubroutineSymbol>(id, name, std::move(parameters)));
    _scopes.back().declare(std::move(name), id);
    return id;
}

std::optional<SymbolId> SymbolTable::lookup(std::string_view name) const
{
    for( auto it = _scopes.rbegin(); it != _scopes.rend(); ++it )
        if (auto id = it->lookup(name))
            return id;

    return std::nullopt;
}

bool SymbolTable::exists(std::string_view name) const
{
    return lookup(name).has_value();
}

const Symbol& SymbolTable::symbol(SymbolId id) const
{
    return *_symbols.at(id - 1);
}

SymbolTable::~SymbolTable() = default;

} // namespace basic
