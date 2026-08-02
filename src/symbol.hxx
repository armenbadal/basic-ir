#pragma once

#include "types.hxx"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace basic {

using SymbolId = unsigned int;

class Symbol {
public:
    enum class Kind {
        Variable,
        Subroutine,
    };

    Symbol(SymbolId id, std::string name);
    virtual ~Symbol() = default;

    SymbolId id() const noexcept;
    std::string_view name() const noexcept;

    virtual Kind kind() const noexcept = 0;

private:
    const std::string _name;
    const SymbolId _id{0};
};

class VariableSymbol : public Symbol {
public:
    enum class Storage {
        Local,
        Parameter,
    };

    VariableSymbol(SymbolId id, std::string name, Type::Ptr type,
        Storage storage = Storage::Local);

    const Type& type() const noexcept;
    Storage storage() const noexcept;

    Symbol::Kind kind() const noexcept override;

private:
    Type::Ptr _type;
    Storage _storage;
};

class SubroutineSymbol : public Symbol {
public:
    SubroutineSymbol(SymbolId id, std::string name, std::vector<SymbolId> parameters);
    ~SubroutineSymbol() override = default;
    const std::vector<SymbolId>& parameters() const;
    void setParameters(std::vector<SymbolId> parameters);
    Symbol::Kind kind() const noexcept override;

private:
    std::vector<SymbolId> _parameters;
};

class SymbolTable {
public:
    SymbolTable();
    ~SymbolTable();

    void openScope();
    void closeScope();

    SymbolId declareVariable(std::string name, Type::Ptr type);
    SymbolId declareParameter(std::string name, Type::Ptr type);
    SymbolId declareSubroutine(std::string name, std::vector<SymbolId> parameters);

    std::optional<SymbolId> lookup(std::string_view name) const;
    bool exists(std::string_view name) const;

    Symbol& symbol(SymbolId id);
    const Symbol& symbol(SymbolId id) const;
    template<typename T>
    const T& symbol(SymbolId id) const
    {
        return dynamic_cast<const T&>(symbol(id));
    }

private:
    class Scope;
    SymbolId nextId();

private:
    std::vector<std::unique_ptr<Symbol>> _symbols;
    std::vector<Scope> _scopes;
    SymbolId _nextId{1};
};

} // namespace basic
