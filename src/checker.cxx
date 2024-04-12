
#include "checker.hxx"
#include "formatters.hxx"

#include <format>
#include <memory>
#include <stdexcept>
#include <string_view>

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
    if( node->expr->type != node->place->type )
        throw TypeError{std::format("{} փոփոխականին վերագրվում է {} արժեք։",
            node->place->type, node->expr->type)};
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
    if( node->condition->isNot(Type::Boolean) )
        throw TypeError{"Ճյուղավորման հրամանի պայմանի տիպը բուլյան չէ։"};

    visit(node->decision);
    visit(node->alternative);
}

//
void Checker::visit(WhilePtr node)
{
    visit(node->condition);
    if( node->condition->isNot(Type::Boolean) )
        throw TypeError{"Պայմանով ցիկլի պայմանի տիպը բուլյան չէ։"};

    visit(node->body);
}

//
void Checker::visit(ForPtr node)
{
    if( node->parameter->isNot(Type::Numeric) )
        throw TypeError{"Պարամետրով ցիկլի պարամետրի տիպը թվային չէ։"};

    visit(node->begin);
    if( node->parameter->isNot(Type::Numeric) )
        throw TypeError{"Պարամետրով ցիկլի պարամետրի սկզբնական արժեքի տիպը թվային չէ։"};

    visit(node->end);
    if( node->parameter->isNot(Type::Numeric) )
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
        if( typeOf(parameters[i]) != arguments[i]->type )
            throw TypeError{std::format("{}-րդ պարամետրի տիպը {} է, իսկ արգումենտի տիպը {} է։",
                    i, typeOf(parameters[i]), arguments[i]->type)};
    }

    node->type = typeOf(node->callee->name);
}

//
void Checker::visit(BinaryPtr node)
{
    visit(node->left);
    visit(node->right);

    Type tyLeft = node->left->type;
    Type tyRight = node->right->type;
    Operation opc = node->opcode;

    // տիպերի ստուգում և որոշում
    if( Type::Boolean == tyLeft && Type::Boolean == tyRight ) {
        const bool allowed = opc == Operation::And ||
                             opc == Operation::Or ||
                             opc == Operation::Eq ||
                             opc == Operation::Ne;
        if( !allowed )
            throw TypeError{std::format("{} գործողությունը կիրառելի չէ տրամաբանական արժեքներին։", opc)};

        node->type = Type::Boolean;
    }
    else if( Type::Numeric == tyLeft && Type::Numeric == tyRight ) {
        const bool notAllowed = opc == Operation::Conc ||
                                opc == Operation::And ||
                                opc == Operation::Or;
        if( notAllowed )
            throw TypeError{std::format("{} գործողությունը կիրառելի չէ թվերին։", opc)};

        if( opc >= Operation::Eq && opc <= Operation::Le )
            node->type = Type::Boolean;
        else
            node->type = Type::Numeric;
    }
    else if( Type::Textual == tyLeft && Type::Textual == tyRight ) {
        if( Operation::Conc == opc )
            node->type = Type::Textual;
        else if( opc >= Operation::Eq && opc <= Operation::Le )
            node->type = Type::Boolean;
        else
            throw TypeError{std::format("{} գործողությունը կիրառելի չէ տեքստերին։", opc)};
    }
    else
        throw TypeError{std::format("{} գործողության երկու կողմերում տարբեր տիպեր են։", opc)};
}

//
void Checker::visit(UnaryPtr node)
{
    visit(node->subexpr);

    if( Operation::Not == node->opcode && node->subexpr->isNot(Type::Boolean) )
        throw TypeError{"Ժխտման գործողության օպերանդը բուլյան չէ։"};
    else
        node->type = Type::Boolean;

    if( Operation::Sub == node->opcode && node->subexpr->isNot(Type::Numeric) )
        throw TypeError{"Բացասման գործողության օպերանդը թվային չէ։"};
    else
        node->type = Type::Numeric;
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
