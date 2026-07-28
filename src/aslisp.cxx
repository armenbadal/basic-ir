#include "aslisp.hxx"

#include <format>
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

void Lisper::emit(Program::Ptr node, std::ostream& os)
{
    _result = {};
    visit(node);
    os << _result;
}

void Lisper::visit(Boolean::Ptr node)
{
    _result = std::format("(basic-boolean {})", node->_value ? "T" : "NIL");
}

void Lisper::visit(Number::Ptr node)
{
    _result = std::format("(basic-number {})", node->_value);
}

void Lisper::visit(Text::Ptr node)
{
    _result = std::format("(basic-text \"{}\")", node->_value);
}

void Lisper::visit(Variable::Ptr node)
{
    _result = std::format("(basic-variable \"{}\")", node->_name);
}

void Lisper::visit(Unary::Ptr node)
{
    visit(node->_operand);
    auto operand = _result;
    _result = std::format("(basic-unary \"{}\" {})", mnemonic[node->_operation], operand);
}

void Lisper::visit(Binary::Ptr node)
{
    visit(node->_left);
    auto ls = _result;
    visit(node->_right);
    auto rs = _result;

    _result = std::format("(basic-binary \"{}\" {} {})", mnemonic[node->_operation], ls, rs);
}

void Lisper::visit(Array::Ptr node)
{
    std::string out = "(basic-array";
    for( auto e : node->_elements ) {
        visit(e);
        out += " " + _result;
    }
    out += ")";
    _result = out;
}

void Lisper::visit(Apply::Ptr node)
{
    std::string out = "(basic-apply \"" + node->_callee + "\"";
    for( auto e : node->_arguments ) {
        visit(e);
        out += " " + _result;
    }
    out += ")";
    _result = out;
}

void Lisper::visit(Let::Ptr node)
{
    visit(node->_expr);
    auto es = _result;

    _result = "(basic-let (basic-variable \"" + node->_variable->_name + "\") "
           + es + ")";
}

void Lisper::visit(Input::Ptr node)
{
    _result = "(basic-input (basic-variable \""
           + node->_variable->_name + "\"))";
}

void Lisper::visit(Print::Ptr node)
{
    visit(node->_expr);
    auto es = _result;
    _result = "(basic-print " + es + ")";
}

void Lisper::visit(Dim::Ptr node)
{
    visit(node->_size);
    auto ss = _result;
    _result = "(basic-dim \"" + node->_name + "\" " + ss + ")";
}

void Lisper::visit(If::Ptr node)
{
    std::string out = "(basic-if";
    visit(node->_condition);
    out += " " + _result;
    visit(node->_decision);
    out += " " + _result;
    if( nullptr != node->_alternative ) {
        visit(node->_alternative);
        out += " " + _result;
    }
    out += ")";
    _result = out;
}

void Lisper::visit(While::Ptr node)
{
    std::string out = "(basic-while";
    visit(node->_condition);
    out += " " + _result;
    visit(node->_body);
    out += " " + _result;
    out += ")";
    _result = out;
}

void Lisper::visit(For::Ptr node)
{
    std::string out = "(basic-for";
    visit(node->_parameter);
    out += " " + _result;
    visit(node->_begin);
    out += " " + _result;
    visit(node->_end);
    out += " " + _result;
    visit(node->_step);
    out += " " + _result;
    visit(node->_body);
    out += " " + _result;
    out += ")";
    _result = out;
}

void Lisper::visit(Call::Ptr node)
{
    std::string out = "(basic-call \"" + (node->_subrCall->_callee) + "\"";
    for( auto e : node->_subrCall->_arguments ) {
        visit(e);
        out += " " + _result;
    }
    out += ")";
    _result = out;
}

void Lisper::visit(Sequence::Ptr node)
{
    std::string out = "(basic-sequence";
    for( auto ei : node->_items ) {
        visit(ei);
        out += " " + _result;
    }
    out += ")";
    _result = out;
}

void Lisper::visit(Subroutine::Ptr node)
{
    std::string out = "(basic-subroutine \"" + node->_name + "\"";
    std::string parlis;
    for( auto& ip : node->_parameters ) {
        parlis.append("\"");
        parlis.append(ip);
        parlis.append("\" ");
    }
    if( !parlis.empty() )
        parlis.pop_back();
    out += " '(" + parlis + ")";
    visit(node->_body);
    out += " " + _result;
    out += ")";
    _result = out;
}

void Lisper::visit(Program::Ptr node)
{
    std::string out = "(basic-program \"" + node->_filename + "\"";
    for( auto si : node->_subroutines ) {
        out += " ";
        visit(si);
        out += _result;
    }
    out += ")\n";

    _result = out;
}

} // basic