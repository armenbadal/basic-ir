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
    os << str(node);
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
    _result = std::format("(basic-unary \"{}\" {})", 
                          mnemonic[node->_operation],
                          str(node->_operand));
}

void Lisper::visit(Binary::Ptr node)
{
    _result = std::format("(basic-binary \"{}\" {} {})", 
                          mnemonic[node->_operation],
                          str(node->_left),
                          str(node->_right));
}

void Lisper::visit(Array::Ptr node)
{
    _result = std::format("(basic-array {})", str(node->_elements));
}

void Lisper::visit(Apply::Ptr node)
{
    _result = std::format("(basic-apply \"{}\" {})",
                          node->_callee, str(node->_arguments));
}

void Lisper::visit(Let::Ptr node)
{
    _result = std::format("(basic-let (basic-variable \"{}\") {})", 
                          node->_variable->_name,
                          str(node->_expr));
}

void Lisper::visit(Input::Ptr node)
{
    _result = std::format("(basic-input (basic-variable \"{}\"))",
                          node->_variable->_name);
}

void Lisper::visit(Print::Ptr node)
{
    _result = std::format("(basic-print {})", str(node->_expr));
}

void Lisper::visit(Dim::Ptr node)
{
    _result = std::format("(basic-dim \"{}\" {})", 
                          node->_name,
                          str(node->_size));
}

void Lisper::visit(If::Ptr node)
{
    auto cond = str(node->_condition);
    auto decision = str(node->_decision);
    auto alt = node->_alternative ? str(node->_alternative) : "";
    _result = "(basic-if " + cond + " " + decision;
    if (!alt.empty())
        _result += " " + alt;
    _result += ")";
}

void Lisper::visit(While::Ptr node)
{
    _result = std::format("(basic-while {} {})",
                          str(node->_condition),
                          str(node->_body));
}

void Lisper::visit(For::Ptr node)
{
    _result = std::format("(basic-for {} {} {} {} {})",
                          str(node->_parameter),
                          str(node->_begin),
                          str(node->_end),
                          str(node->_step),
                          str(node->_body));
}

void Lisper::visit(Call::Ptr node)
{
    _result = std::format("(basic-call \"{}\" {})",
                          node->_subrCall->_callee,
                          str(node->_subrCall->_arguments));
}

void Lisper::visit(Sequence::Ptr node)
{
    _result = std::format("(basic-sequence {})", str(node->_items));
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
    auto subs = str(node->_subroutines);
    _result = "(basic-program \"" + node->_filename + "\"";
    if (!subs.empty()) _result += " " + subs;
    _result += ")\n";
}

} // basic