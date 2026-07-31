#pragma once

#include "type.hxx"

#include <string>
#include <string_view>

namespace basic {

using SymbolId = unsigned int;

class Symbol {
public:
    Symbol(std::string name);
    virtual ~Symbol() = default;

    SymbolId id() const;
    std::string_view name() const;

private:
    std::string _name;
    SymbolId _id{0};
};

class VariableSymbol : public Symbol {};
class ParameterSymbol : public Symbol {};
class SubroutineSymbol : public Symbol {};

class SymbolTable {
public:

    void beginSubroutine(const std::string& name);
    void endSubroutine();

    SymbolId declareParameter(std::string_view name, Type type);
    SymbolId declareArray(std::string_view name, Type elementType);
    SymbolId defineVariable(std::string_view name, Type type);
    std::optional<SymbolId> lookup(std::string_view name) const;
    const Symbol& symbol(SymbolId id) const;

};

} // namespace basic