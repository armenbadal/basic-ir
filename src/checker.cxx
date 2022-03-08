
#include "checker.hxx"

#include <iostream>

namespace basic {
//
class TypeError : public std::exception {
public:
    TypeError( const std::string& mes )
        : message(mes)
    {}

    const char* what() const noexcept override
    {
        return message.c_str();
    }

private:
    std::string message;
};

    
//
std::optional<std::string> Checker::check(AstNodePtr node)
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
    if( node->expr->type != node->varptr->type ) {
        std::string mes = toString(node->varptr->type)
            + " փոփոխականին վերագրվում է "
            + toString(node->expr->type) + " արժեք։";
        throw TypeError(mes);
    }
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
    visit(node->decision);
    visit(node->alternative);

    if( Type::Number != node->condition->type )
        throw TypeError("Ճյուղավորման հրամանի պայմանի տիպը թվային չէ։");
}

//
void Checker::visit(WhilePtr node)
{
    visit(node->condition);
    if( Type::Number != node->condition->type )
        throw TypeError("Պայմանով ցիկլի պայմանի տիպը թվային չէ։");

    visit(node->body);
}

//
void Checker::visit(ForPtr node)
{
    if( Type::Number != node->parameter->type )
        throw TypeError("Պարամետրով ցիկլի պարամետրի տիպը թվային չէ։");

    visit(node->begin);
    if( Type::Number != node->begin->type )
        throw TypeError("Պարամետրով ցիկլի պարամետրի սկզբնական արժեքի տիպը թվային չէ։");

    visit(node->end);
    if( Type::Number != node->end->type )
        throw TypeError("Պարամետրով ցիկլի պարամետրի վերջնական արժեքի տիպը թվային չէ։");

    if( 0 == node->step->value )
        throw TypeError("պարամետրով ցիկլի քայլը զրո է։");
    
    visit(node->body);
}

//
void Checker::visit(CallPtr node)
{
    // Խուժան քայլ։ Քանի որ Call-ը նույն Apply-ն է, և
    // տիպերի ստուգումը կատարվում է Apply օբյեկտի համար,
    // պետք է ժամանակավորապես փոխել կանչվող ենթածրագրի
    // hasValue դաշտը։
    auto proc = node->subrcall->procptr;

    bool hv = proc->hasValue;
    proc->hasValue = true;

    visit(node->subrcall);

    // վերականգնել հին արժեքը
    proc->hasValue = hv;
}

//
void Checker::visit(ApplyPtr node)
{
    // Ստուգել, որ կանչվող ենթածրագիրը արժեք վերադարձնի։
    if( !node->procptr->hasValue )
        throw TypeError(node->procptr->name + " ենթածրագիրն արժեք չի վերադարձնում։");

    auto& parameters = node->procptr->parameters;
    auto& arguments = node->arguments;

    if( parameters.size() != arguments.size() )
        throw TypeError("Պարամետրերի ու արգումենտների քանակները հավասար չեն։");

    for( int i = 0; i < arguments.size(); ++i ) {
        // TODO: check also each parameter
        if( typeOf(parameters[i]) != arguments[i]->type ) {
            std::string mes = std::to_string(i) + "-րդ պարամետրի տիպը "
                + toString(typeOf(parameters[i])) + " է, իսկ արգումենտի տիպը "
                + toString(arguments[i]->type) + " է։";
            throw TypeError(mes);
        }
    }

    node->type = typeOf(node->procptr->name);
}

//
void Checker::visit(BinaryPtr node)
{
    visit(node->subexpro);
    visit(node->subexpri);

    Type tyo = node->subexpro->type;
    Type tyi = node->subexpri->type;
    Operation opc = node->opcode;

    // տիպերի ստուգում և որոշում
    if( Type::Number == tyo && Type::Number == tyi ) {
        if( Operation::Conc == opc )
            throw TypeError("'&' գործողությունը կիրառելի չէ թվերին։");

        node->type = Type::Number;
    }
    else if( Type::Text == tyo && Type::Text == tyi ) {
        if( Operation::Conc == opc )
            node->type = Type::Text;
        else if( opc >= Operation::Eq && opc <= Operation::Le )
            node->type = Type::Number;
        else
            throw TypeError("'" + toString(opc) + "' գործողությունը կիրառելի չէ տեքստերին։");
    }
    else
        throw TypeError("'" + toString(opc) + "' գործողության երկու կողմերում տարբեր տիպեր են։");
}

//
void Checker::visit(UnaryPtr node)
{
    visit(node->subexpr);

    if( Type::Number != node->subexpr->type )
        throw TypeError("Ունար գործողության օպերանդը թվային չէ։");

    node->type = Type::Number;
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

void Checker::visit(AstNodePtr node)
{
    if( nullptr != node )
        AstVisitor::visit(node);
}
}
