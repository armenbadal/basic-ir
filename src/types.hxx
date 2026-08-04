#pragma once

#include <memory>
#include <optional>
#include <string_view>

namespace basic {

class Type {
public:
    enum class Kind {
        Nothing,
        Scalar,
        Array,
    };

    virtual ~Type() = default;
    virtual std::string_view name() const = 0;
    virtual bool equals(const Type&) const noexcept = 0;
    virtual Kind kind() const = 0;

    using Ptr = std::shared_ptr<const Type>;
};

bool operator==(const Type& lhs, const Type& rhs) noexcept;

class Types {
public:
    static Type::Ptr nothing();
    static Type::Ptr real();
    static Type::Ptr boolean();
    static Type::Ptr text();
    static Type::Ptr array(Type::Ptr elementType);
    static Type::Ptr fromKeyword(std::string_view keyword);
};


class Nothing : public Type {
public:
    std::string_view name() const override
    {
        return "NOTHING";
    }

    bool equals(const Type& other) const noexcept override
    {
        return other.kind() == Type::Kind::Nothing;
    }

    Type::Kind kind() const noexcept override
    {
        return Type::Kind::Nothing;
    }
};


class ScalarType : public Type {
public:
    enum class Kind {
        Real,
        Boolean,
        Text,
    };

    explicit ScalarType(Kind kind) : _kind{kind} {}
    std::string_view name() const override;
    bool equals(const Type&) const noexcept override;
    Type::Kind kind() const noexcept override
    {
        return Type::Kind::Scalar;
    }

private:
    const Kind _kind;
};


class ArrayType : public Type {
public:
    explicit ArrayType(Type::Ptr elementType)
        : _elementType{std::move(elementType)}
    {}

    std::string_view name() const override
    {
        return "ARRAY";
    }

    const Type& elementType() const noexcept
    {
        return *_elementType;
    }

    Type::Ptr elementTypePtr() const noexcept
    {
        return _elementType;
    }

    bool equals(const Type&) const noexcept override;

    Type::Kind kind() const noexcept override
    {
        return Type::Kind::Array;
    }

private:
    const Type::Ptr _elementType;
};

} // namespace basic
