#include "aslisp.hxx"

#include <format>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace basic {

std::unordered_map<Operation, std::string> mnemonic{
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
    os << visit(*node) << '\n';
}

std::string Lisper::visit(Boolean& node)
{
    return std::format("(basic-boolean {})", node._value ? "T" : "NIL");
}

std::string Lisper::visit(Number& node)
{
    return std::format("(basic-number {})", node._value);
}

std::string Lisper::visit(Text& node)
{
    return std::format("(basic-text \"{}\")", node._value);
}

std::string Lisper::visit(Variable& node)
{
    return std::format("(basic-variable \"{}\")", node._name);
}

std::string Lisper::visit(Unary& node)
{
    return std::format("(basic-unary \"{}\" {})", 
                       mnemonic[node._operation],
                       visit(*node._operand));
}

std::string Lisper::visit(Binary& node)
{
    return std::format("(basic-binary \"{}\" {} {})", 
                       mnemonic[node._operation],
                       visit(*node._left),
                       visit(*node._right));
}

std::string Lisper::visit(Array& node)
{
    return std::format("(basic-array {})", visitVector(node._elements));
}

std::string Lisper::visit(Apply& node)
{
    return std::format("(basic-apply \"{}\" {})",
                       node._callee,
                       visitVector(node._arguments));
}

std::string Lisper::visit(Let& node)
{
    return std::format("(basic-let (basic-variable \"{}\") {})", 
                       node._variable->_name,
                       visit(*node._expr));
}

std::string Lisper::visit(Dim& node)
{
    return std::format("(basic-dim \"{}\" {} \"{}\")",
                       node._name, node._size ? visit(*node._size) : "NIL", node._type);
}

std::string Lisper::visit(If& node)
{
    auto branches = visitVector(node._branches);
    auto alternative = node._alternative ? " " + visit(*node._alternative) : "";
    return std::format("(basic-if {}{})", branches, alternative);
}

std::string Lisper::visit(If::IfThen& node)
{
    return "(basic-if-then " + visit(*node._condition) + " " + visit(*node._decision) + ")";
}

std::string Lisper::visit(While& node)
{
    return std::format("(basic-while {} {})", visit(*node._condition), visit(*node._body));
}

std::string Lisper::visit(For& node)
{
    return std::format("(basic-for {} {} {} {} {})",
                       visit(*node._parameter),
                       visit(*node._begin),
                       visit(*node._end),
                       visit(*node._step),
                       visit(*node._body));
}

std::string Lisper::visit(Call& node)
{
    return "(basic-call \"" + node._subrCall->_callee + "\"" + spaced(node._subrCall->_arguments) + ")";
}

std::string Lisper::visit(Sequence& node)
{
    return "(basic-sequence" + spaced(node._items) + ")";
}

std::string Lisper::visit(Subroutine& node)
{
    return "(basic-subroutine \"" + node._name + "\" '(" + visitVector(node._parameters) + ") " + visit(*node._body) + ")";
}

std::string Lisper::visit(Program& node)
{
    return "(basic-program" + spaced(node._subroutines) + ")";
}

} // basic