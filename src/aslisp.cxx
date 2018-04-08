
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
bool Lisper::emitLisp( const std::string& osn )
{
    osp = std::make_shared<std::ofstream>(osn);
    visit(node);
    osp->close();

    return true;
}

///
void Lisper::visit( NumberPtr node )
{
    ooo() << "(basic-number " << node->value << ")";
}

///
void Lisper::visit( TextPtr node )
{
    ooo() << "(basic-text \"" << node->value << "\")";
}

///
void Lisper::visit( VariablePtr node )
{
    ooo() << "(basic-variable \"" << node->name << "\")";
}

///
void Lisper::visit( UnaryPtr node )
{
    ooo() << "(basic-unary \"" << mnemonic[node->opcode] << "\" ";
    visit(node->subexpr);
    ooo() << ")";
}

///
void Lisper::visit( BinaryPtr node )
{
    ooo() << "(basic-binary \"" << mnemonic[node->opcode] << "\"";
    ++indent;
    visit(node->subexpro);
    visit(node->subexpri);
    --indent;
    ooo() << ")";
}

///
void Lisper::visit( ApplyPtr node )
{
    ooo() << "(basic-apply \"" << node->procptr->name << "\"";
    ++indent;
    for( auto e : node->arguments )
        visit(e);
    --indent;
    ooo() << ")";
}

///
void Lisper::visit( LetPtr node )
{
    ooo() << "(basic-let (basic-variable \""
          << node->varptr->name << "\") ";
    ++indent;
    visit(node->expr);
    --indent;
    ooo() << ")";
}

///
void Lisper::visit( InputPtr node )
{
    ooo() << "(basic-input (basic-variable \"" << node->prompt 
        << "\") \"" << node->varptr->name << "\")";
}

///
void Lisper::visit( PrintPtr node )
{
    ooo() << "(basic-print";
    ++indent;
    visit(node->expr);
    --indent;
    ooo() << ")";
}

///
void Lisper::visit( IfPtr node )
{
    ooo() << "(basic-if";
    ++indent;
    visit(node->condition);
    visit(node->decision);
    if( nullptr != node->alternative )
        visit(node->alternative);
    ooo() << ")";
    --indent;
}

///
void Lisper::visit(  WhilePtr node )
{
    ooo() << "(basic-while";
    ++indent;
    visit(node->condition);
    visit(node->body);
    ooo() << ")";
    --indent;
}

///
void Lisper::visit( ForPtr node )
{
    ooo() << "(basic-for";
    ++indent;
    visit(node->parameter);
    visit(node->begin);
    visit(node->end);
    visit(node->step);
    visit(node->body);
    ooo() << ")";
    --indent;
}

///
void Lisper::visit( CallPtr node )
{
    ooo() << "(basic-call \"" << (node->subrcall->procptr->name) << "\"";
    ++indent;
    for( auto e : node->subrcall->arguments )
        visit(e);
    ooo() << ")";
    --indent;
}

///
void Lisper::visit( SequencePtr node )
{
    ooo() << "(basic-sequence";
    ++indent;
    for( auto ei : node->items )
        visit(ei);
    ooo() << ")";
    --indent;
}

///
void Lisper::visit( SubroutinePtr node )
{
    ooo() << "(basic-subroutine \"" << node->name << "\"";
    ++indent;
    std::string parlis = "";
    for( auto& ip : node->parameters ) {
        parlis.append("\"");
        parlis.append(ip);
        parlis.append("\" ");
    }
    if( !parlis.empty() )
        parlis.pop_back();
    ooo() << std::endl << std::string(2 * indent, ' ')
        << "'(" << parlis << ")";
    visit(node->body);
    ooo() << ")";
    --indent;
}

///
void Lisper::visit( ProgramPtr node )
{
    ooo() << "(basic-program \"" << node->filename << "\"";
    ++indent;
    for( auto si : node->members )
        if( !si->isBuiltIn )
            visit(si);
    --indent;
    ooo() << ")" << std::endl;
}

///
void Lisper::visit( AstNodePtr node )
{
    if( nullptr == node )
        return;

    ooo() << std::endl << std::string(2 * indent, ' ');

    AstVisitor::visit(node);
}

///
std::ostream& Lisper::ooo()
{
    return *osp;
}
} // basic

