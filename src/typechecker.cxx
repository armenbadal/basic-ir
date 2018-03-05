
#include "typechecker.hxx"

namespace basic {
//
bool TypeChecker::check(AstNode* node)
{
    try {
        visitAstNode(node);
    }
    catch(...){
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
    if( node->expr->type != node->varptr->type )
        throw "Type error 1";
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

    if( node->condition->type != Type::Number )
        throw "Type error 2";
}

//
void TypeChecker::visitWhile(While* node)
{
    visitAstNode(node->condition);
    visitAstNode(node->condition);

    if( node->condition->type != Type::Number )
        throw "Type error 3";
}

//
void TypeChecker::visitFor(For* node)
{
    visitAstNode(node->begin);
    visitAstNode(node->end);
    visitAstNode(node->body);

    if( Type::Number != node->parameter->type )
        throw "Type error 4";

    if( Type::Number != node->begin->type )
        throw "Type error 5";

    if (Type::Number != node->end->type)
        throw "Type error 6";
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
        throw "Type error 7";

    for( int i = 0; i < arguments.size(); ++i )
        if( typeOf(parameters[i]) != arguments[i]->type )
            throw "Type error 8";

    node->type = typeOf(node->procptr->name);
}

//
void TypeChecker::visitBinary(Binary* node)
{
    visitAstNode(node->subexpro);
    visitAstNode(node->subexpri);

    //
}

//
void TypeChecker::visitUnary(Unary* node)
{
    visitAstNode(node->subexpr);

    if( Type::Number != node->type )
        throw "Type error 9";

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
