
#include "aslisp.hxx"

#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>

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
bool Lisper::emitLisp(ProgramPtr node, const std::filesystem::path& file)
{
    std::ofstream os{file};
    if( !os )
        return false;

    visit(node);
    
    os.close();

    return true;
}


// ///
// void Lisper::visit(NodePtr node)
// {
//     if( nullptr == node )
//         return;

//     os << std::endl << std::string(2 * indent, ' ');

//     ASTVisitor::visit(node);
// }

std::string Lisper::visit(ExpressionPtr node)
{
    return dispatch(node);
}

std::string Lisper::visit(BooleanPtr node)
{
    return "(basic-boolean " 
           + std::string{node->value ? "T" : "NIL"}
           + ")";
}

///
std::string Lisper::visit(NumberPtr node)
{
    return "(basic-number " + std::to_string(node->value) + ")";
}

///
std::string Lisper::visit(TextPtr node)
{
    return "(basic-text \"" + node->value + "\")";
}

///
std::string Lisper::visit(VariablePtr node)
{
    return "(basic-variable \"" + node->name + "\")";
}

///
std::string Lisper::visit(UnaryPtr node)
{
    return "(basic-unary \"" + mnemonic[node->opcode] + "\" "
           + visit(node->subexpr) + ")";
}

///
std::string Lisper::visit(BinaryPtr node)
{
    ++indent;
    auto ls = visit(node->left);
    auto rs = visit(node->right);
    --indent;

    return "(basic-binary \"" + mnemonic[node->opcode] + "\" "
           + ls + " " + rs + ")";
}

///
std::string Lisper::visit(ApplyPtr node)
{
    std::string out;

    ++indent;
    for( auto e : node->arguments )
        out += visit(e);
    --indent;

    return "(basic-apply \"" + node->callee->name + "\"" + out + ")";
}

//
std::string Lisper::visit(StatementPtr node)
{
    return dispatch(node);
}

///
std::string Lisper::visit(LetPtr node)
{
    ++indent;
    auto es = visit(node->expr);
    --indent;

    return "(basic-let (basic-variable \"" + node->place->name + "\") " + es + ")";
}

///
std::string Lisper::visit(InputPtr node)
{
    return "(basic-input (basic-variable " 
           + visit(node->prompt)
           + " \"" + node->place->name + "\")";
}

///
std::string Lisper::visit(PrintPtr node)
{
    ++indent;
    auto es = visit(node->expr);
    --indent;
    return "(basic-print" + es + ")";
}

///
std::string Lisper::visit(IfPtr node)
{
    std::string out = "(basic-if";
    ++indent;
    out += visit(node->condition);
    out += visit(node->decision);
    if( nullptr != node->alternative )
        out += visit(node->alternative);
    --indent;
    out += ")";
    return out;
}

///
std::string Lisper::visit(WhilePtr node)
{
    std::string out = "(basic-while";
    ++indent;
    out += visit(node->condition);
    out += visit(node->body);
    --indent;
    out += ")";
    return out;
}

///
std::string Lisper::visit(ForPtr node)
{
    std::string out = "(basic-for";
    ++indent;
    out += visit(node->parameter);
    out += visit(node->begin);
    out += visit(node->end);
    out += visit(node->step);
    out += visit(node->body);
    --indent;
    out += ")";
    return out;
}

///
std::string Lisper::visit(CallPtr node)
{
    std::string out = "(basic-call \"" + (node->subrCall->callee->name) + "\"";
    ++indent;
    for( auto e : node->subrCall->arguments )
        out += visit(e);
    --indent;
    out += ")";
    return out;
}

///
std::string Lisper::visit(SequencePtr node)
{
    std::string out = "(basic-sequence";
    ++indent;
    for( auto ei : node->items )
        out += visit(ei);
    --indent;
    out += ")";
    return out;
}

///
std::string Lisper::visit(SubroutinePtr node)
{
    std::string out = "(basic-subroutine \"" + node->name + "\"";
    ++indent;
    std::string parlis = "";
    for( auto& ip : node->parameters ) {
        parlis.append("\"");
        parlis.append(ip);
        parlis.append("\" ");
    }
    if( !parlis.empty() )
        parlis.pop_back();
    out += "\n" + std::string(2 * indent, ' ') + "'(" + parlis + ")";
    out += visit(node->body);
    --indent;
    out += ")";
    return out;
}

///
std::string Lisper::visit(ProgramPtr node)
{
    std::string out = "(basic-program \"" + node->filename + "\"";
    ++indent;
    for( auto si : node->members )
        if( !si->isBuiltIn )
            out += visit(si);
    --indent;
    out += ")\n";

    return out;
}

} // basic

