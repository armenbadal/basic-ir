#include "types.hxx"

namespace basic {

bool operator==(const Type& lhs, const Type& rhs) noexcept
{
    return lhs.equals(rhs);
}

Type::Ptr Types::nothing()
{
    static Type::Ptr instance = std::make_shared<Nothing>();
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

Type::Ptr Types::text()
{
    static Type::Ptr instance = std::make_shared<ScalarType>(ScalarType::Kind::Text);
    return instance;
}

Type::Ptr Types::array(Type::Ptr elementType)
{
    return std::make_shared<ArrayType>(std::move(elementType));
}

Type::Ptr Types::fromKeyword(std::string_view keyword)
{
    if( keyword == "REAL" )
        return real();
    if( keyword == "TEXT" )
        return text();
    if( keyword == "BOOL" )
        return boolean();
    return nothing();
}


std::string_view ScalarType::name() const
{
    switch( _kind ) {
        case Kind::Real:
            return "REAL";
        case Kind::Boolean:
            return "BOOLEAN";
        case Kind::Text:
            return "TEXT";
    }
    return "UNKNOWN";
}

bool ScalarType::equals(const Type& other) const noexcept
{
    if( other.kind() != Type::Kind::Scalar )
        return false;

    auto const& rhs = static_cast<const ScalarType&>(other);
    return _kind == rhs._kind;
}

bool ArrayType::equals(const Type& other) const noexcept
{
    if( other.kind() != Type::Kind::Array )
        return false;

    auto const& rhs = static_cast<const ArrayType&>(other);
    return _elementType->equals(*rhs._elementType);
}

} // namespace basic
