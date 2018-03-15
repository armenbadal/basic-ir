
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

//
Number::Number( double vl )
    : value(vl)
{
    kind = NodeKind::Number;
    type = Type::Number;
}

//
Text::Text( const std::string& vl )
    : value(vl)
{
    kind = NodeKind::Text;
    type = Type::Text;
}

//
Variable::Variable( const std::string& nm )
    : name(nm)
{
    kind = NodeKind::Variable;
    type = typeOf(name);
}

//
Unary::Unary( Operation op, ExpressionPtr ex )
    : opcode(op), subexpr(ex)
{
    kind = NodeKind::Unary;
    type = Type::Number;
}

//
Binary::Binary( Operation op, ExpressionPtr exo, ExpressionPtr exi )
    : opcode(op), subexpro(exo), subexpri(exi)
{
    kind = NodeKind::Binary;
}

//
Apply::Apply( SubroutinePtr sp, const std::vector<ExpressionPtr>& ags )
    : procptr(sp), arguments(ags)
{
    kind = NodeKind::Apply;
}

//
Sequence::Sequence()
{
    kind = NodeKind::Sequence;
}

//
Input::Input( const std::string& pr, VariablePtr vp )
    : prompt(pr), varptr(vp)
{
    kind = NodeKind::Input;
}

//
Print::Print( ExpressionPtr ex )
    : expr(ex)
{
    kind = NodeKind::Print;
}

//
Let::Let( VariablePtr vp, ExpressionPtr ex )
    : varptr(vp), expr(ex)
{
    kind = NodeKind::Let;
}

//
If::If( ExpressionPtr co, StatementPtr de, StatementPtr al )
    : condition(co), decision(de), alternative(al)
{
    kind = NodeKind::If;
}

//
While::While( ExpressionPtr co, StatementPtr bo )
    : condition(co), body(bo)
{
    kind = NodeKind::While;
}

//
For::For( VariablePtr pr, ExpressionPtr be, ExpressionPtr en,
          NumberPtr st, StatementPtr bo )
    : parameter(pr), begin(be), end(en), step(st), body(bo)
{
    kind = NodeKind::For;
}

//
Call::Call( SubroutinePtr sp, const std::vector<ExpressionPtr>& as )
    : subrcall(std::make_shared<Apply>(sp, as))
{
    kind = NodeKind::Call;
}

//
Subroutine::Subroutine( const std::string& nm, const std::vector<std::string>& ps )
    : name(nm), parameters(ps)
{
    kind = NodeKind::Subroutine;
    for( auto& ps : parameters )
        locals.push_back(std::make_shared<Variable>(ps));
}

//
Program::Program( const std::string& fn )
    : filename(fn)
{
    kind = NodeKind::Program;
}
} // basic
