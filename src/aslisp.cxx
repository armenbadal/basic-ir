
#include "aslisp.hxx"

#include <map>
#include <sstream>
#include <string>

namespace basic {
///
std::map<Operation, std::string> mnemonic{
    { Operation::None, "?" },
    { Operation::Add,  "ADD" },
    { Operation::Sub,  "SUB" },
    { Operation::Mul,  "MUL" },
    { Operation::Div,  "DIV" },
    { Operation::Mod,  "MOD" },
    { Operation::Pow,  "POW" },
    { Operation::Eq,   "EQ" },
    { Operation::Ne,   "NE" },
    { Operation::Gt,   "GT" },
    { Operation::Ge,   "GE" },
    { Operation::Lt,   "LT" },
    { Operation::Le,   "LE" },
    { Operation::And,  "AND" },
    { Operation::Or,   "OR" },
    { Operation::Not,  "NOT" },
    { Operation::Conc, "CONC" }
};

///
bool Lisper::asLisp( AstNodePtr node )
{
    visitAstNode(node);
    return true;
}

///
void Lisper::visitNumber( NumberPtr node )
{
    ooo << "(basic-number " << node->value << ")";
}

///
void Lisper::visitText( TextPtr node )
{
    ooo << "(basic-text \"" << node->value << "\")";
}

///
void Lisper::visitVariable( VariablePtr node )
{
    ooo << "(basic-variable \"" << node->name << "\")";
}

///
void Lisper::visitUnary( UnaryPtr node )
{
    ooo << "(basic-unary \"" << mnemonic[node->opcode] << "\" ";
    visitAstNode(node->subexpr);
    ooo << ")";
}

///
void Lisper::visitBinary( BinaryPtr node )
{
    ooo << "(basic-binary \"" << mnemonic[node->opcode] << "\"";
    ++indent;
    visitAstNode(node->subexpro);
    visitAstNode(node->subexpri);
    --indent;
    ooo << ")";
}

///
void Lisper::visitApply( ApplyPtr node )
{
    ooo << "(basic-apply \"" << node->procptr->name << "\"";
    ++indent;
    for( auto e : node->arguments )
        visitAstNode(e);
    --indent;
    ooo << ")";
}

///
void Lisper::visitLet( LetPtr node )
{
    ooo << "(basic-let (basic-variable \""
        << node->varptr->name << "\") ";
    ++indent;
    visitAstNode(node->expr);
    --indent;
    ooo << ")";
}

///
void Lisper::visitInput( InputPtr node )
{
    ooo << "(basic-input (basic-variable \"" << node->prompt 
        << "\") \"" << node->varptr->name << "\")";
}

///
void Lisper::visitPrint( PrintPtr node )
{
    ooo << "(basic-print";
    ++indent;
    visitAstNode(node->expr);
    --indent;
    ooo << ")";
}

///
void Lisper::visitIf( IfPtr node )
{
    ooo << "(basic-if";
    ++indent;
    visitAstNode(node->condition);
    visitAstNode(node->decision);
    if( nullptr != node->alternative )
        visitAstNode(node->alternative);
    ooo << ")";
    --indent;
}

///
void Lisper::visitWhile(  WhilePtr node )
{
    ooo << "(basic-while";
    ++indent;
    visitAstNode(node->condition);
    visitAstNode(node->body);
    ooo << ")";
    --indent;
}

///
void Lisper::visitFor( ForPtr node )
{
    ooo << "(basic-for";
    ++indent;
    visitAstNode(node->parameter);
    visitAstNode(node->begin);
    visitAstNode(node->end);
    visitAstNode(node->step);
    visitAstNode(node->body);
    ooo << ")";
    --indent;
}

///
void Lisper::visitCall( CallPtr node )
{
    ooo << "(basic-call \"" << (node->subrcall->procptr->name) << "\"";
    ++indent;
    for( auto e : node->subrcall->arguments )
        visitAstNode(e);
    ooo << ")";
    --indent;
}

///
void Lisper::visitSequence( SequencePtr node )
{
    ooo << "(basic-sequence";
    ++indent;
    for( auto ei : node->items )
        visitAstNode(ei);
    ooo << ")";
    --indent;
}

///
void Lisper::visitSubroutine( SubroutinePtr node )
{
    ooo << "(basic-subroutine \"" << node->name << "\"";
    ++indent;
    std::string parlis = "";
    for( auto& ip : node->parameters ) {
        parlis.append("\"");
        parlis.append(ip);
        parlis.append("\" ");
    }
    if( !parlis.empty() )
        parlis.pop_back();
    ooo << std::endl << std::string(2 * indent, ' ')
        << "'(" << parlis << ")";
    visitAstNode(node->body);
    ooo << ")";
    --indent;
}

///
void Lisper::visitProgram( ProgramPtr node )
{
    ooo << "(basic-program \"" << node->filename << "\"";
    ++indent;
    for( auto si : node->members )
        if( !si->isBuiltIn )
            visitAstNode(si);
    --indent;
    ooo << ")" << std::endl;
}

void Lisper::visitAstNode( AstNodePtr node )
{
    if( nullptr == node )
        return;

    ooo << std::endl << std::string(2 * indent, ' ');

    AstVisitor::visitAstNode(node);
}
} // basic
