
#include "ast.hxx"

#include <iostream>
#include <map>

namespace basic {
//
std::string toString( Operation opc )
{
    static std::map<Operation,std::string> names{
        { Operation::None, "None" },
        { Operation::Add, "+" },
        { Operation::Sub, "-" },
        { Operation::Mul, "*" },
        { Operation::Div, "/" },
        { Operation::Mod, "\\" },
        { Operation::Pow, "^" },
        { Operation::Eq, "=" },
        { Operation::Ne, "<>" },
        { Operation::Gt, ">" },
        { Operation::Ge, ">=" },
        { Operation::Lt, "<" },
        { Operation::Le, "<=" },
        { Operation::And, "AND" },
        { Operation::Or, "OR" },
        { Operation::Not, "NOT" },
        { Operation::Conc, "&" }
    };
    return names[opc];
}

//
Type typeOf( const std::string& nm )
{
    return nm.back() == '$' ? Type::Text : Type::Number;
}

std::string toString( Type vl )
{
    if( Type::Number == vl )
        return "NUMBER";

    if( Type::Text == vl )
        return "TEXT";

    return "VOID";
}
} // basic
