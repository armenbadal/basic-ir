
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
bool Lisper::emitLisp(AstNodePtr node, const std::filesystem::path& file)
{
    os = std::ofstream{file};
    if( !os )
        return false;

    visit(node);
    
    os.close();

    return true;
}

void Lisper::visit(BooleanPtr node)
{
    os << "(basic-number " << (node->value ? "T" : "NIL") << ")";
}

///
void Lisper::visit(NumberPtr node)
{
    os << "(basic-number " << node->value << ")";
}

///
void Lisper::visit(TextPtr node)
{
    os << "(basic-text \"" << node->value << "\")";
}

///
void Lisper::visit(VariablePtr node)
{
    os << "(basic-variable \"" << node->name << "\")";
}

///
void Lisper::visit(UnaryPtr node)
{
    os << "(basic-unary \"" << mnemonic[node->opcode] << "\" ";
    visit(node->subexpr);
    os << ")";
}

///
void Lisper::visit(BinaryPtr node)
{
    os << "(basic-binary \"" << mnemonic[node->opcode] << "\"";
    ++indent;
    visit(node->subexpro);
    visit(node->subexpri);
    --indent;
    os << ")";
}

///
void Lisper::visit(ApplyPtr node)
{
    os << "(basic-apply \"" << node->procptr->name << "\"";
    ++indent;
    for( auto e : node->arguments )
        visit(e);
    --indent;
    os << ")";
}

///
void Lisper::visit(LetPtr node)
{
    os << "(basic-let (basic-variable \""
          << node->varptr->name << "\") ";
    ++indent;
    visit(node->expr);
    --indent;
    os << ")";
}

///
void Lisper::visit(InputPtr node)
{
    os << "(basic-input (basic-variable \"" << node->prompt 
        << "\") \"" << node->varptr->name << "\")";
}

///
void Lisper::visit(PrintPtr node)
{
    os << "(basic-print";
    ++indent;
    visit(node->expr);
    --indent;
    os << ")";
}

///
void Lisper::visit(IfPtr node)
{
    os << "(basic-if";
    ++indent;
    visit(node->condition);
    visit(node->decision);
    if( nullptr != node->alternative )
        visit(node->alternative);
    os << ")";
    --indent;
}

///
void Lisper::visit(WhilePtr node)
{
    os << "(basic-while";
    ++indent;
    visit(node->condition);
    visit(node->body);
    os << ")";
    --indent;
}

///
void Lisper::visit(ForPtr node)
{
    os << "(basic-for";
    ++indent;
    visit(node->parameter);
    visit(node->begin);
    visit(node->end);
    visit(node->step);
    visit(node->body);
    os << ")";
    --indent;
}

///
void Lisper::visit(CallPtr node)
{
    os << "(basic-call \"" << (node->subrcall->procptr->name) << "\"";
    ++indent;
    for( auto e : node->subrcall->arguments )
        visit(e);
    os << ")";
    --indent;
}

///
void Lisper::visit(SequencePtr node)
{
    os << "(basic-sequence";
    ++indent;
    for( auto ei : node->items )
        visit(ei);
    os << ")";
    --indent;
}

///
void Lisper::visit(SubroutinePtr node)
{
    os << "(basic-subroutine \"" << node->name << "\"";
    ++indent;
    std::string parlis = "";
    for( auto& ip : node->parameters ) {
        parlis.append("\"");
        parlis.append(ip);
        parlis.append("\" ");
    }
    if( !parlis.empty() )
        parlis.pop_back();
    os << std::endl << std::string(2 * indent, ' ')
        << "'(" << parlis << ")";
    visit(node->body);
    os << ")";
    --indent;
}

///
void Lisper::visit(ProgramPtr node)
{
    os << "(basic-program \"" << node->filename << "\"";
    ++indent;
    for( auto si : node->members )
        if( !si->isBuiltIn )
            visit(si);
    --indent;
    os << ")" << std::endl;
}

///
void Lisper::visit(AstNodePtr node)
{
    if( nullptr == node )
        return;

    os << std::endl << std::string(2 * indent, ' ');

    AstVisitor::visit(node);
}

} // basic

