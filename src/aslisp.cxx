#include "aslisp.hxx"

#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace basic {

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

bool Lisper::emitLisp(Program::Ptr node, const std::filesystem::path& file)
{
    std::ofstream os{file};
    if( !os )
        return false;

    visit(node);
    os << _result;
    os.close();

    return true;
}

void Lisper::visit(Boolean::Ptr node)
{
    _result = "(basic-boolean "
           + std::string{node->_value ? "T" : "NIL"}
           + ")";
}

void Lisper::visit(Number::Ptr node)
{
    _result = "(basic-number " + std::to_string(node->_value) + ")";
}

void Lisper::visit(Text::Ptr node)
{
    _result = "(basic-text \"" + node->_value + "\")";
}

void Lisper::visit(Variable::Ptr node)
{
    _result = "(basic-variable \"" + node->_name + "\")";
}

void Lisper::visit(Unary::Ptr node)
{
    visit(node->_operand);
    auto operand = _result;
    _result = "(basic-unary \"" + mnemonic[node->_operation] + "\" "
           + operand + ")";
}

void Lisper::visit(Binary::Ptr node)
{
    ++indent;
    visit(node->_left);
    auto ls = _result;
    visit(node->_right);
    auto rs = _result;
    --indent;

    _result = "(basic-binary \"" + mnemonic[node->_operation] + "\" "
           + ls + " " + rs + ")";
}

void Lisper::visit(Apply::Ptr node)
{
    std::string out;

    ++indent;
    for( auto e : node->_arguments ) {
        visit(e);
        out += _result;
    }
    --indent;

    _result = "(basic-apply \"" + node->_callee + "\"" + out + ")";
}

void Lisper::visit(Let::Ptr node)
{
    ++indent;
    visit(node->_expr);
    auto es = _result;
    --indent;

    _result = "(basic-let (basic-variable \"" + node->_variable->_name + "\") " + es + ")";
}

void Lisper::visit(Input::Ptr node)
{
    _result = "(basic-input (basic-variable \""
           + node->_variable->_name + "\"))";
}

void Lisper::visit(Print::Ptr node)
{
    ++indent;
    visit(node->_expr);
    auto es = _result;
    --indent;
    _result = "(basic-print" + es + ")";
}

void Lisper::visit(If::Ptr node)
{
    std::string out = "(basic-if";
    ++indent;
    visit(node->_condition);
    out += _result;
    visit(node->_decision);
    out += _result;
    if( nullptr != node->_alternative ) {
        visit(node->_alternative);
        out += _result;
    }
    --indent;
    out += ")";
    _result = out;
}

void Lisper::visit(While::Ptr node)
{
    std::string out = "(basic-while";
    ++indent;
    visit(node->_condition);
    out += _result;
    visit(node->_body);
    out += _result;
    --indent;
    out += ")";
    _result = out;
}

void Lisper::visit(For::Ptr node)
{
    std::string out = "(basic-for";
    ++indent;
    visit(node->_parameter);
    out += _result;
    visit(node->_begin);
    out += _result;
    visit(node->_end);
    out += _result;
    visit(node->_step);
    out += _result;
    visit(node->_body);
    out += _result;
    --indent;
    out += ")";
    _result = out;
}

void Lisper::visit(Call::Ptr node)
{
    std::string out = "(basic-call \"" + (node->_subrCall->_callee) + "\"";
    ++indent;
    for( auto e : node->_subrCall->_arguments ) {
        visit(e);
        out += _result;
    }
    --indent;
    out += ")";
    _result = out;
}

void Lisper::visit(Sequence::Ptr node)
{
    std::string out = "(basic-sequence";
    ++indent;
    for( auto ei : node->_items ) {
        visit(ei);
        out += _result;
    }
    --indent;
    out += ")";
    _result = out;
}

void Lisper::visit(Subroutine::Ptr node)
{
    std::string out = "(basic-subroutine \"" + node->_name + "\"";
    ++indent;
    std::string parlis;
    for( auto& ip : node->_parameters ) {
        parlis.append("\"");
        parlis.append(ip);
        parlis.append("\" ");
    }
    if( !parlis.empty() )
        parlis.pop_back();
    out += "\n" + std::string(2 * indent, ' ') + "'(" + parlis + ")";
    visit(node->_body);
    out += _result;
    --indent;
    out += ")";
    _result = out;
}

void Lisper::visit(Program::Ptr node)
{
    std::string out = "(basic-program \"" + node->_filename + "\"";
    ++indent;
    for( auto si : node->_subroutines ) {
        visit(si);
        out += _result;
    }
    --indent;
    out += ")\n";

    _result = out;
}

void Lisper::visit(Dim::Ptr node)
{
    _result = {};
}

} // basic
