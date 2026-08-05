#pragma once

#include "types.hxx"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace basic {

using SymbolId = unsigned int;
constexpr SymbolId UnknownSymbol = 0;

// Ծրագրում հանդիպող անունները ստեղծում են սիմվոլներ
class Symbol {
public:
    // դրանք կարող են երկու բան ցույց տալ. փոփոխական (նաև զանգված) կամ ենթածրագիր
    enum class Kind {
        Variable,
        Subroutine,
    };

    Symbol(SymbolId id, std::string name)
        : _name{std::move(name)}, _id{id}
    {}
    virtual ~Symbol() = default;

    SymbolId id() const noexcept
    {
        return _id;
    }

    std::string_view name() const noexcept
    {
        return _name;
    }

    virtual Kind kind() const noexcept = 0;

private:
    const std::string _name;
    const SymbolId _id{0};
};

// Փոփոխական (կամ զանգված) ցույց տվող սիմվոլ
class VariableSymbol : public Symbol {
public:
    // կարող է ունենալ երեք դեր. 
    enum class Storage {
        Local,  // լոկալ փոփոխական, DIM֊ով հայտարարված
        Parameter, // ենթածրագրի պարամետր
        ReturnValue // անբացահտ ստեծված արժեք՝ ենթածրագրից վերադարձնելու համար
    };

    VariableSymbol(SymbolId id, std::string name, Type::Ptr type, Storage storage = Storage::Local)
        : Symbol{id, std::move(name)}
        , _type{std::move(type)}
        , _storage{storage}
    {}

    const Type& type() const noexcept
    {
        return *_type;
    }

    const Type::Ptr& typePtr() const noexcept
    {
        return _type;
    }

    Storage storage() const noexcept
    {
        return _storage;
    }

    Symbol::Kind kind() const noexcept override
    {
        return Symbol::Kind::Variable;
    }

private:
    Type::Ptr _type;
    Storage _storage;
};

// Ենթածրագրի սիմվոլ
class SubroutineSymbol : public Symbol {
public:
    SubroutineSymbol(SymbolId id, std::string name, std::vector<Type::Ptr> parameterTypes, std::optional<Type::Ptr> returnType)
        : Symbol{id, std::move(name)}
        , _parameterTypes{std::move(parameterTypes)}
        , _returnType{std::move(returnType)}
    {}

    const std::vector<Type::Ptr>& parameterTypes() const noexcept
    {
        return _parameterTypes;
    }

    std::optional<Type::Ptr> returnType() const noexcept
    {
        return _returnType;
    }

    Symbol::Kind kind() const noexcept override
    {
        return Symbol::Kind::Subroutine;
    }

private:
    std::vector<Type::Ptr> _parameterTypes;
    std::optional<Type::Ptr> _returnType;
};


// Սիմվոլների աղյուսակ
class SymbolTable {
public:
    SymbolTable();
    ~SymbolTable() = default;

    void openScope();
    void closeScope();

    SymbolId declareVariable(std::string name, Type::Ptr type, VariableSymbol::Storage storage = VariableSymbol::Storage::Local);
    SymbolId declareParameter(std::string name, Type::Ptr type);
    SymbolId declareSubroutine(std::string name, std::vector<Type::Ptr> parameterTypes, std::optional<Type::Ptr> returnType);

    std::optional<SymbolId> lookup(std::string_view name) const;
    // Փնտրում է ենթածրագրի սիմվոլը՝ անտեսելով նույն անունով փոփոխականները
    std::optional<SymbolId> lookupSubroutine(std::string_view name) const;
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
