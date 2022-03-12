
#include "ast.hxx"

#include <iostream>
#include <map>

namespace basic {
//
const std::string toString(Operation opc)
{
    static std::map<Operation,std::string> names{
        { Operation::None, "None" },
        { Operation::Add,   "+" },
        { Operation::Sub,   "-" },
        { Operation::Mul,   "*" },
        { Operation::Div,   "/" },
        { Operation::Mod,   "\\" },
        { Operation::Pow,   "^" },
        { Operation::Eq,   "=" },
        { Operation::Ne,   "<>" },
        { Operation::Gt,   ">" },
        { Operation::Ge,   ">=" },
        { Operation::Lt,   "<" },
        { Operation::Le,   "<=" },
        { Operation::And,  "AND" },
        { Operation::Or,   "OR" },
        { Operation::Not,  "NOT" },
        { Operation::Conc, "&" }
    };
    return names[opc];
}

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

const std::string toString(Type ty)
{
    if( Type::Boolean == ty )
        return "BOOLEAN";

    if( Type::Numeric == ty )
        return "NUMBER";

    if( Type::Textual == ty )
        return "TEXT";

    return "VOID";
}
} // basic
