
#include "typechecker.hxx"

#include <iostream>

namespace basic {
//
class TypeError : public std::exception {
public:
    TypeError(const std::string& mes)
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
bool TypeChecker::check(AstNode* node)
{
    try {
        visitAstNode(node);
    }
    catch(TypeError& e) {
        std::cerr << "Տիպի սխալ։ " << e.what() << std::endl;
        return false;
    }

    return true;
}

//
void TypeChecker::visitProgram(Program* node)
{
    for( auto si : node->members )
        visitAstNode(si);
}

//
void TypeChecker::visitSubroutine(Subroutine* node)
{
    visitAstNode(node->body);
}

//
void TypeChecker::visitSequence(Sequence* node)
{
    for( auto si : node->items )
        visitAstNode(si);
}

//
void TypeChecker::visitLet(Let* node)
{
    visitAstNode(node->expr);
    if( node->expr->type != node->varptr->type ) {
        std::string mes = toString(node->varptr->type)
            + " փոփոխականին վերագրվում է "
            + toString(node->expr->type) + " արժեք։";
        throw TypeError(mes);
    }
}

//
void TypeChecker::visitInput(Input* node)
{}

//
void TypeChecker::visitPrint(Print* node)
{
    visitAstNode(node->expr);
}

//
void TypeChecker::visitIf(If* node)
{
    visitAstNode(node->condition);
    visitAstNode(node->decision);
    visitAstNode(node->alternative);

    if(Type::Number != node->condition->type )
        throw TypeError("Ճյուղավորման հրամանի պայմանի տիպը թվային չէ։");
}

//
void TypeChecker::visitWhile(While* node)
{
    visitAstNode(node->condition);
    visitAstNode(node->condition);

    if(Type::Number != node->condition->type )
        throw TypeError("Պայմանով ցիկլի պայմանի տիպը թվային չէ։");
}

//
void TypeChecker::visitFor(For* node)
{
    visitAstNode(node->begin);
    visitAstNode(node->end);
    visitAstNode(node->body);

    if( Type::Number != node->parameter->type )
        throw TypeError("Պարամետրով ցիկլի պարամետրի տիպը թվային չէ։");

    if( Type::Number != node->begin->type )
        throw TypeError("Պարամետրով ցիկլի պարամետրի սկզբնական արժեքի տիպը թվային չէ։");

    if (Type::Number != node->end->type)
        throw TypeError("Պարամետրով ցիկլի պարամետրի վերջնական արժեքի տիպը թվային չէ։");
}

//
void TypeChecker::visitCall(Call* node)
{
    visitApply(node->subrcall);
}

//
void TypeChecker::visitApply(Apply* node)
{
    auto& parameters = node->procptr->parameters;
    auto& arguments = node->arguments;

    if( parameters.size() != arguments.size() )
        throw TypeError("Պարամետրերի ու արգումենտների քանակները հավասար չեն։");

    for( int i = 0; i < arguments.size(); ++i )
        if( typeOf(parameters[i]) != arguments[i]->type ) {
            std::string mes = std::to_string(i) + "-րդ պարամետրի տիպը "
                + toString(typeOf(parameters[i])) + " է, իսկ արգումենտի տիպը "
                + toString(arguments[i]->type) + " է։";
            throw TypeError(mes);
        }

    node->type = typeOf(node->procptr->name);
}

//
void TypeChecker::visitBinary(Binary* node)
{
    visitAstNode(node->subexpro);
    visitAstNode(node->subexpri);

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

    throw TypeError("'" + toString(opc) + "' գործողության երկու կողմերում տարբեր տիպեր են։");
}

//
void TypeChecker::visitUnary(Unary* node)
{
    visitAstNode(node->subexpr);

    if( Type::Number != node->type )
        throw TypeError("Ունար գործողության օպերանդը թվային չէ։");

    node->type = Type::Number;
}

//
void TypeChecker::visitVariable(Variable* node)
{
    // ճիշտ տիպը նախորոշված է
}

//
void TypeChecker::visitText(Text* node)
{
    // ճիշտ տիպը նախորոշված է
}

//
void TypeChecker::visitNumber(Number* node)
{
    // ճիշտ տիպը նախորոշված է
}
}