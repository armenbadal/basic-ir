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
        case NodeKind::Dim:
            return derived().visit(static_cast<Dim&>(node));
        case NodeKind::Let:
            return derived().visit(static_cast<Let&>(node));
        case NodeKind::Input:
            return derived().visit(static_cast<Input&>(node));
        case NodeKind::Print:
            return derived().visit(static_cast<Print&>(node));
        case NodeKind::If:
            return derived().visit(static_cast<If&>(node));
        case NodeKind::While:
            return derived().visit(static_cast<While&>(node));
        case NodeKind::For:
            return derived().visit(static_cast<For&>(node));
        case NodeKind::Call:
            return derived().visit(static_cast<Call&>(node));
        case NodeKind::Array:
            return derived().visit(static_cast<Array&>(node));
        case NodeKind::Apply:
            return derived().visit(static_cast<Apply&>(node));
        case NodeKind::Binary:
            return derived().visit(static_cast<Binary&>(node));
        case NodeKind::Unary:
            return derived().visit(static_cast<Unary&>(node));
        case NodeKind::Variable:
            return derived().visit(static_cast<Variable&>(node));
        case NodeKind::Text:
            return derived().visit(static_cast<Text&>(node));
        case NodeKind::Number:
            return derived().visit(static_cast<Number&>(node));
        case NodeKind::Boolean:
            return derived().visit(static_cast<Boolean&>(node));
        default:
            return {};
        }

        return {};
    }

private:
    Derived& derived() { return static_cast<Derived&>(*this); }
};

} // namespace basic
