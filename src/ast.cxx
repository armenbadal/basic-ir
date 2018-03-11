
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
Unary::Unary( Operation op, std::shared_ptr<Expression> ex )
    : opcode(op), subexpr(ex)
{
    kind = NodeKind::Unary;
    type = Type::Number;
}

//
Binary::Binary( Operation op, std::shared_ptr<Expression> exo, std::shared_ptr<Expression> exi )
    : opcode(op), subexpro(exo), subexpri(exi)
{
    kind = NodeKind::Binary;
}

//
Apply::Apply( std::shared_ptr<Subroutine> sp, const std::vector<std::shared_ptr<Expression>>& ags )
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
Input::Input( const std::string& pr, std::shared_ptr<Variable> vp )
    : prompt(pr), varptr(vp)
{
    kind = NodeKind::Input;
}

//
Print::Print( std::shared_ptr<Expression> ex )
    : expr(ex)
{
    kind = NodeKind::Print;
}

//
Let::Let( std::shared_ptr<Variable> vp, std::shared_ptr<Expression> ex )
    : varptr(vp), expr(ex)
{
    kind = NodeKind::Let;
}

//
If::If( std::shared_ptr<Expression> co, std::shared_ptr<Statement> de, std::shared_ptr<Statement> al )
    : condition(co), decision(de), alternative(al)
{
    kind = NodeKind::If;
}

//
While::While( std::shared_ptr<Expression> co, std::shared_ptr<Statement> bo )
    : condition(co), body(bo)
{
    kind = NodeKind::While;
}

//
For::For( std::shared_ptr<Variable> pr, std::shared_ptr<Expression> be,
          std::shared_ptr<Expression> en, std::shared_ptr<Expression> st,
          std::shared_ptr<Statement> bo )
    : parameter(pr), begin(be), end(en), step(st), body(bo)
{
    kind = NodeKind::For;
}

//
Call::Call( std::shared_ptr<Subroutine> sp, const std::vector<std::shared_ptr<Expression>>& as )
    : subrcall(std::make_shared<Apply>(sp, as))
{
    kind = NodeKind::Call;
}

//
Subroutine::Subroutine( const std::string& nm, const std::vector<std::string>& ps )
    : name(nm), parameters(ps)
{
    kind = NodeKind::Subroutine;
    locals.push_back(std::make_shared<Variable>(name));
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
