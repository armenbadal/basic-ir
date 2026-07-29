#pragma once

#include "ast.hxx"

namespace basic {

template<typename Derived, typename ReturnType = void>
class ASTVisitor {
protected:
    ReturnType visit(Node& node)
    {
        switch (node.kind) {
        case NodeKind::Sequence:
            return derived().visit(static_cast<Sequence&>(node));
            break;
        case NodeKind::Dim:
            return derived().visit(static_cast<Dim&>(node));
            break;
        case NodeKind::Let:
            return derived().visit(static_cast<Let&>(node));
            break;
        case NodeKind::Input:
            return derived().visit(static_cast<Input&>(node));
            break;
        case NodeKind::Print:
            return derived().visit(static_cast<Print&>(node));
            break;
        case NodeKind::If:
            return derived().visit(static_cast<If&>(node));
            break;
        case NodeKind::While:
            return derived().visit(static_cast<While&>(node));
            break;
        case NodeKind::For:
            return derived().visit(static_cast<For&>(node));
            break;
        case NodeKind::Call:
            return derived().visit(static_cast<Call&>(node));
            break;
        case NodeKind::Array:
            return derived().visit(static_cast<Array&>(node));
            break;
        case NodeKind::Apply:
            return derived().visit(static_cast<Apply&>(node));
            break;
        case NodeKind::Binary:
            return derived().visit(static_cast<Binary&>(node));
            break;
        case NodeKind::Unary:
            return derived().visit(static_cast<Unary&>(node));
            break;
        case NodeKind::Variable:
            return derived().visit(static_cast<Variable&>(node));
            break;
        case NodeKind::Text:
            return derived().visit(static_cast<Text&>(node));
            break;
        case NodeKind::Number:
            return derived().visit(static_cast<Number&>(node));
            break;
        case NodeKind::Boolean:
            return derived().visit(static_cast<Boolean&>(node));
            break;
        default:
            break;
        }
    }

private:
    Derived& derived() { return static_cast<Derived&>(*this); }
};

} // namespace basic
