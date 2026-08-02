#include "checker.hxx"
#include "formatters.hxx"

#include <format>
#include <memory>
#include <stdexcept>
#include <string_view>

namespace {

char typeOfName(std::string_view name)
{
    if( name.back() == '?' )
        return 'B';

    if( name.back() == '$' )
        return 'T';

    return 'N';
}

char exprType(const basic::Expression::Ptr& e)
{
    using namespace basic;
    switch( e->kind ) {
        case NodeKind::Boolean: return 'B';
        case NodeKind::Number:  return 'N';
        case NodeKind::Text:    return 'T';
        case NodeKind::Variable: {
            auto v = std::static_pointer_cast<Variable>(e);
            return typeOfName(v->_name);
        }
        case NodeKind::Apply: {
            auto a = std::static_pointer_cast<Apply>(e);
            return typeOfName(a->_callee);
        }
        case NodeKind::Unary: {
            auto u = std::static_pointer_cast<Unary>(e);
            return u->_operation == Operation::Not ? 'B' : 'N';
        }
        case NodeKind::Binary: {
            auto b = std::static_pointer_cast<Binary>(e);
            auto l = exprType(b->_left);
            auto r = exprType(b->_right);
            if( l != r ) return 'V';
            if( b->_operation >= Operation::Eq && b->_operation <= Operation::Le )
                return 'B';
            if( b->_operation == Operation::Conc )
                return 'T';
            if( b->_operation == Operation::And || b->_operation == Operation::Or )
                return 'B';
            return l;
        }
        default: return 'V';
    }
}

} // anonymous namespace

namespace basic {

class TypeError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

std::optional<std::string> Checker::check(Program::Ptr node)
{
    subroutines.clear();
    for( auto& sub : node->_subroutines )
        subroutines[sub->_name] = sub;

    try {
        visit(node);
    }
    catch( TypeError& e ) {
        return "Տիպի սխալ։ " + std::string{e.what()};
    }

    return std::nullopt;
}

void Checker::visit(Program::Ptr node)
{
    for( auto si : node->_subroutines )
        visit(si);
}

void Checker::visit(Subroutine::Ptr node)
{
    if( "Main" == node->_name )
        if( !node->_parameters.empty() )
            throw TypeError("Main ենթածրագիրը պարամետրեր չպետք է ունենա։");

    visit(node->_body);
}

void Checker::visit(Sequence::Ptr node)
{
    for( auto si : node->_items )
        visit(si);
}

void Checker::visit(Dim::Ptr node)
{
}

void Checker::visit(Let::Ptr node)
{
    visit(node->_expr);
    auto pt = typeOfName(node->_variable->_name);
    auto et = exprType(node->_expr);
    if( pt != et )
        throw TypeError{std::format("{} փոփոխականին վերագրվում է {} արժեք։", pt, et)};
}

void Checker::visit(If::Ptr node)
{
    visit(node->_condition);
    if( exprType(node->_condition) != 'B' )
        throw TypeError{"Ճյուղավորման հրամանի պայմանի տիպը բուլյան չէ։"};

    visit(node->_decision);
    visit(node->_alternative);
}

void Checker::visit(While::Ptr node)
{
    visit(node->_condition);
    if( exprType(node->_condition) != 'B' )
        throw TypeError{"Պայմանով ցիկլի պայմանի տիպը բուլյան չէ։"};

    visit(node->_body);
}

void Checker::visit(For::Ptr node)
{
    auto pt = typeOfName(node->_parameter->_name);
    if( pt != 'N' )
        throw TypeError{"Պարամետրով ցիկլի պարամետրի տիպը թվային չէ։"};

    visit(node->_begin);
    if( exprType(node->_begin) != 'N' )
        throw TypeError{"Պարամետրով ցիկլի պարամետրի սկզբնական արժեքի տիպը թվային չէ։"};

    visit(node->_end);
    if( exprType(node->_end) != 'N' )
        throw TypeError{"Պարամետրով ցիկլի պարամետրի վերջնական արժեքի տիպը թվային չէ։"};

    if( 0 == node->_step->_value )
        throw TypeError{"Պարամետրով ցիկլի քայլը զրո է։"};

    visit(node->_body);
}

void Checker::visit(Call::Ptr node)
{
    visit(node->_subrCall);
}

void Checker::visit(Array::Ptr)
{
}

void Checker::visit(Apply::Ptr node)
{
    auto it = subroutines.find(node->_callee);
    if( it == subroutines.end() )
        return;

    auto& callee = it->second;
    auto& params = callee->_parameters;
    auto& args = node->_arguments;

    if( params.size() != args.size() )
        throw TypeError{"Պարամետրերի ու արգումենտների քանակները հավասար չեն։"};

    for( int i = 0; i < args.size(); ++i ) {
        auto pt = typeOfName(params[i]);
        auto at = exprType(args[i]);
        if( pt != at )
            throw TypeError{std::format("{}-րդ պարամետրի տիպը {} է, իսկ արգումենտի տիպը {} է։",
                    i, pt, at)};
    }
}

void Checker::visit(Binary::Ptr node)
{
    visit(node->_left);
    visit(node->_right);

    auto tyLeft = exprType(node->_left);
    auto tyRight = exprType(node->_right);
    Operation opc = node->_operation;

    if( tyLeft != tyRight )
        throw TypeError{std::format("{} գործողության երկու կողմերում տարբեր տիպեր են։", opc)};

    if( 'B' == tyLeft ) {
        const bool allowed = opc == Operation::And ||
                             opc == Operation::Or ||
                             opc == Operation::Eq ||
                             opc == Operation::Ne;
        if( !allowed )
            throw TypeError{std::format("{} գործողությունը կիրառելի չէ տրամաբանական արժեքներին։", opc)};
    }
    else if( 'N' == tyLeft ) {
        const bool notAllowed = opc == Operation::Conc ||
                                opc == Operation::And ||
                                opc == Operation::Or;
        if( notAllowed )
            throw TypeError{std::format("{} գործողությունը կիրառելի չէ թվերին։", opc)};
    }
    else if( 'T' == tyLeft ) {
        if( opc != Operation::Conc && !(opc >= Operation::Eq && opc <= Operation::Le) )
            throw TypeError{std::format("{} գործողությունը կիրառելի չէ տեքստերին։", opc)};
    }
}

void Checker::visit(Unary::Ptr node)
{
    visit(node->_operand);

    auto st = exprType(node->_operand);

    if( Operation::Not == node->_operation && st != 'B' )
        throw TypeError{"Ժխտման գործողության օպերանդը բուլյան չէ։"};

    if( Operation::Sub == node->_operation && st != 'N' )
        throw TypeError{"Բացասման գործողության օպերանդը թվային չէ։"};
}

void Checker::visit(Variable::Ptr node)
{
}

void Checker::visit(Text::Ptr node)
{
}

void Checker::visit(Number::Ptr node)
{
}

void Checker::visit(Boolean::Ptr node)
{
}

} // namespace basic
