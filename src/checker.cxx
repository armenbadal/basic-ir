
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

char exprType(const basic::ExpressionPtr& e)
{
    switch( e->kind ) {
        case basic::NodeKind::Boolean: return 'B';
        case basic::NodeKind::Number:  return 'N';
        case basic::NodeKind::Text:    return 'T';
        case basic::NodeKind::Variable: {
            auto v = std::static_pointer_cast<basic::Variable>(e);
            return typeOfName(v->name);
        }
        case basic::NodeKind::Apply: {
            auto a = std::static_pointer_cast<basic::Apply>(e);
            if( a->callee )
                return typeOfName(a->callee->name);
            return 'V';
        }
        case basic::NodeKind::Unary: {
            auto u = std::static_pointer_cast<basic::Unary>(e);
            return u->opcode == basic::Operation::Not ? 'B' : 'N';
        }
        case basic::NodeKind::Binary: {
            auto b = std::static_pointer_cast<basic::Binary>(e);
            auto l = exprType(b->left);
            auto r = exprType(b->right);
            if( l != r ) return 'V';
            if( b->opcode >= basic::Operation::Eq && b->opcode <= basic::Operation::Le )
                return 'B';
            if( b->opcode == basic::Operation::Conc )
                return 'T';
            if( b->opcode == basic::Operation::And || b->opcode == basic::Operation::Or )
                return 'B';
            return l;
        }
        default: return 'V';
    }
}

} // anonymous namespace

namespace basic {
//
class TypeError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

    
//
std::optional<std::string> Checker::check(ProgramPtr node)
{
    try {
        visit(node);
    }
    catch( TypeError& e ) {
        return "Տիպի սխալ։ " + std::string{e.what()};
    }

    return std::nullopt;
}

//
void Checker::visit(ProgramPtr node)
{
    for( auto si : node->members )
        visit(si);
}

//
void Checker::visit(SubroutinePtr node)
{
    if( "Main" == node->name )
        if( !node->parameters.empty() )
            throw TypeError("Main ենթածրագիրը պարամետրեր չպետք է ունենա։");
    
    visit(node->body);
}

void Checker::visit(StatementPtr node)
{
    if( node != nullptr )
    dispatch(node);
}

//
void Checker::visit(SequencePtr node)
{
    for( auto si : node->items )
        visit(si);
}

//
void Checker::visit(LetPtr node)
{
    visit(node->expr);
    auto pt = typeOfName(node->place->name);
    auto et = exprType(node->expr);
    if( pt != et )
        throw TypeError{std::format("{} փոփոխականին վերագրվում է {} արժեք։",
            pt, et)};
}

//
void Checker::visit(InputPtr node)
{}

//
void Checker::visit(PrintPtr node)
{
    visit(node->expr);
}

//
void Checker::visit(IfPtr node)
{
    visit(node->condition);
    if( exprType(node->condition) != 'B' )
        throw TypeError{"Ճյուղավորման հրամանի պայմանի տիպը բուլյան չէ։"};

    visit(node->decision);
    visit(node->alternative);
}

//
void Checker::visit(WhilePtr node)
{
    visit(node->condition);
    if( exprType(node->condition) != 'B' )
        throw TypeError{"Պայմանով ցիկլի պայմանի տիպը բուլյան չէ։"};

    visit(node->body);
}

//
void Checker::visit(ForPtr node)
{
    auto pt = typeOfName(node->parameter->name);
    if( pt != 'N' )
        throw TypeError{"Պարամետրով ցիկլի պարամետրի տիպը թվային չէ։"};

    visit(node->begin);
    if( exprType(node->begin) != 'N' )
        throw TypeError{"Պարամետրով ցիկլի պարամետրի սկզբնական արժեքի տիպը թվային չէ։"};

    visit(node->end);
    if( exprType(node->end) != 'N' )
        throw TypeError{"Պարամետրով ցիկլի պարամետրի վերջնական արժեքի տիպը թվային չէ։"};

    if( 0 == node->step->value )
        throw TypeError{"Պարամետրով ցիկլի քայլը զրո է։"};
    
    visit(node->body);
}

//
void Checker::visit(CallPtr node)
{
    // Խուժան քայլ։ Քանի որ Call-ը նույն Apply-ն է, և
    // տիպերի ստուգումը կատարվում է Apply օբյեկտի համար,
    // պետք է ժամանակավորապես փոխել կանչվող ենթածրագրի
    // hasValue դաշտը։
    auto proc = node->subrCall->callee;

    bool hv = proc->hasValue;
    proc->hasValue = true;

    visit(node->subrCall);

    // վերականգնել հին արժեքը
    proc->hasValue = hv;
}

//
void Checker::visit(ExpressionPtr node)
{
    dispatch(node);
}

//
void Checker::visit(ApplyPtr node)
{
    // Ստուգել, որ կանչվող ենթածրագիրը արժեք վերադարձնի։
    if( !node->callee->hasValue )
        throw TypeError{std::format("{} ենթածրագիրն արժեք չի վերադարձնում։", node->callee->name)};

    auto& parameters = node->callee->parameters;
    auto& arguments = node->arguments;

    if( parameters.size() != arguments.size() )
        throw TypeError{"Պարամետրերի ու արգումենտների քանակները հավասար չեն։"};

    for( int i = 0; i < arguments.size(); ++i ) {
        // TODO: check also each parameter
        auto pt = typeOfName(parameters[i]);
        auto at = exprType(arguments[i]);
        if( pt != at )
            throw TypeError{std::format("{}-րդ պարամետրի տիպը {} է, իսկ արգումենտի տիպը {} է։",
                    i, pt, at)};
    }
}

//
void Checker::visit(BinaryPtr node)
{
    visit(node->left);
    visit(node->right);

    auto tyLeft = exprType(node->left);
    auto tyRight = exprType(node->right);
    Operation opc = node->opcode;

    if( tyLeft != tyRight )
        throw TypeError{std::format("{} գործողության երկու կողմերում տարբեր տիպեր են։", opc)};

    if( 'B' == tyLeft && 'B' == tyRight ) {
        const bool allowed = opc == Operation::And ||
                             opc == Operation::Or ||
                             opc == Operation::Eq ||
                             opc == Operation::Ne;
        if( !allowed )
            throw TypeError{std::format("{} գործողությունը կիրառելի չէ տրամաբանական արժեքներին։", opc)};
    }
    else if( 'N' == tyLeft && 'N' == tyRight ) {
        const bool notAllowed = opc == Operation::Conc ||
                                opc == Operation::And ||
                                opc == Operation::Or;
        if( notAllowed )
            throw TypeError{std::format("{} գործողությունը կիրառելի չէ թվերին։", opc)};
    }
    else if( 'T' == tyLeft && 'T' == tyRight ) {
        if( opc != Operation::Conc && !(opc >= Operation::Eq && opc <= Operation::Le) )
            throw TypeError{std::format("{} գործողությունը կիրառելի չէ տեքստերին։", opc)};
    }
}

//
void Checker::visit(UnaryPtr node)
{
    visit(node->subexpr);

    auto st = exprType(node->subexpr);

    if( Operation::Not == node->opcode && st != 'B' )
        throw TypeError{"Ժխտման գործողության օպերանդը բուլյան չէ։"};

    if( Operation::Sub == node->opcode && st != 'N' )
        throw TypeError{"Բացասման գործողության օպերանդը թվային չէ։"};
}

//
void Checker::visit(VariablePtr node)
{
    // ճիշտ տիպը նախորոշված է
}

//
void Checker::visit(TextPtr node)
{
    // ճիշտ տիպը նախորոշված է
}

//
void Checker::visit(NumberPtr node)
{
    // ճիշտ տիպը նախորոշված է
}

//
void Checker::visit(BooleanPtr node)
{
    // ճիշտ տիպը նախորոշված է
}

} // namespace basic
