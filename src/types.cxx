#include "types.hxx"

namespace basic {

bool operator==(const Type& lhs, const Type& rhs) noexcept
{
    return lhs.equals(rhs);
}

Type::Ptr Types::integer()
{
    static Type::Ptr instance = std::make_shared<ScalarType>(ScalarType::Kind::Integer);
    return instance;
}

Type::Ptr Types::real()
{
    static Type::Ptr instance = std::make_shared<ScalarType>(ScalarType::Kind::Real);
    return instance;
}

Type::Ptr Types::boolean()
{
    static Type::Ptr instance = std::make_shared<ScalarType>(ScalarType::Kind::Boolean);
    return instance;
}

Type::Ptr Types::array(Type::Ptr elementType)
{
    return std::make_shared<ArrayType>(std::move(elementType));
}


ScalarType::ScalarType(Kind kind)
    : _kind{kind}
{}

std::string_view ScalarType::name() const
{
    switch(_kind) {
        case Kind::Integer:
            return "INTEGER";
        case Kind::Real:
            return "REAL";
        case Kind::Boolean:
            return "BOOLEAN";
    }
    return "UNKNOWN";
}

bool ScalarType::equals(const Type& other) const noexcept
{
    if (other.kind() != Type::Kind::Scalar)
        return false;

    auto const& rhs = static_cast<const ScalarType&>(other);
    return _kind == rhs._kind;
}

Type::Kind ScalarType::kind() const noexcept
{
    return Type::Kind::Scalar;
}


ArrayType::ArrayType(Type::Ptr elementType)
    : _elementType{std::move(elementType)}
{}

std::string_view ArrayType::name() const
{
    return "ARRAY";
}

const Type& ArrayType::elementType() const noexcept
{
    return *_elementType;
}

bool ArrayType::equals(const Type& other) const noexcept
{
    if (other.kind() != Type::Kind::Array)
        return false;

    auto const& rhs = static_cast<const ArrayType&>(other);
    return _elementType->equals(*rhs._elementType);
}

Type::Kind ArrayType::kind() const noexcept
{
    return Type::Kind::Array;
}

} // namespace basic
