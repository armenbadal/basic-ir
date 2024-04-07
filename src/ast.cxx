#include "ast.hxx"

#include <map>

namespace basic {

// 
bool Expression::is(Type ty)
{
    return type == ty;
}

bool Expression::isNot(Type ty)
{
    return !is(type);
}

//
Type typeOf(std::string_view name)
{
    if( name.back() == '?' )
        return Type::Boolean;

    if( name.back() == '$' )
        return Type::Textual;
        
    return Type::Numeric;
}

} // basic
