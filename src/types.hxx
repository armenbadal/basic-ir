#pragma once

#include <memory>
#include <string_view>

namespace basic {

class Type {
public:
    enum class Kind {
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
    static Type::Ptr real();
    static Type::Ptr boolean();
    static Type::Ptr text();
    static Type::Ptr array(Type::Ptr elementType);
};

class ScalarType : public Type {
public:
    enum class Kind {
        Real,
        Boolean,
        Text,
    };

    explicit ScalarType(Kind kind);
    std::string_view name() const override;
    bool equals(const Type&) const noexcept override;
    Type::Kind kind() const noexcept override;

private:
    const Kind _kind;
};

class ArrayType : public Type {
public:
    explicit ArrayType(Type::Ptr elementType);
    std::string_view name() const override;
    const Type& elementType() const noexcept;
    Type::Ptr elementTypePtr() const noexcept;
    bool equals(const Type&) const noexcept override;
    Type::Kind kind() const noexcept override;

private:
    const Type::Ptr _elementType;
};

} // namespace basic
