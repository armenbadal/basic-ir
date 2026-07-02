#pragma once

#include "ast.hxx"

#include <type_traits>

namespace basic {

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    
protected:
    virtual void visit(Program::Ptr node) = 0;
    virtual void visit(Subroutine::Ptr node) = 0;

    virtual void visit(Statement::Ptr node) = 0;
    virtual void visit(Sequence::Ptr node) = 0;
    virtual void visit(Dim::Ptr node) = 0;
    virtual void visit(Let::Ptr node) = 0;
    virtual void visit(Input::Ptr node) = 0;
    virtual void visit(Print::Ptr node) = 0;
    virtual void visit(If::Ptr node) = 0;
    virtual void visit(While::Ptr node) = 0;
    virtual void visit(For::Ptr node) = 0;
    virtual void visit(Call::Ptr node) = 0;

    virtual void visit(Expression::Ptr node) = 0;
    virtual void visit(Apply::Ptr node) = 0;
    virtual void visit(Binary::Ptr node) = 0;
    virtual void visit(Unary::Ptr node) = 0;
    virtual void visit(Variable::Ptr node) = 0;
    virtual void visit(Text::Ptr node) = 0;
    virtual void visit(Number::Ptr node) = 0;
    virtual void visit(Boolean::Ptr node) = 0;
};


class ASTVisitorBase : public ASTVisitor {
protected:
    void visit(const Statement::Ptr node) override
    {
        switch (node->kind) {
        case NodeKind::Sequence:
            visit(std::static_pointer_cast<Sequence>(node));
            break;
        case NodeKind::Dim:
            visit(std::static_pointer_cast<Dim>(node));
            break;
        case NodeKind::Let:
            visit(std::static_pointer_cast<Let>(node));
            break;
        case NodeKind::Input:
            visit(std::static_pointer_cast<Input>(node));
            break;
        case NodeKind::Print:
            visit(std::static_pointer_cast<Print>(node));
            break;
        case NodeKind::If:
            visit(std::static_pointer_cast<If>(node));
            break;
        case NodeKind::While:
            visit(std::static_pointer_cast<While>(node));
            break;
        case NodeKind::For:
            visit(std::static_pointer_cast<For>(node));
            break;
        case NodeKind::Call:
            visit(std::static_pointer_cast<Call>(node));
            break;
        default:
            // Handle unknown statement kind if necessary
            break;
        }
    }

    void visit(Expression::Ptr node) override
    {
        switch (node->kind) {
        case NodeKind::Apply:
            visit(std::static_pointer_cast<Apply>(node));
            break;
        case NodeKind::Binary:
            visit(std::static_pointer_cast<Binary>(node));
            break;
        case NodeKind::Unary:
            visit(std::static_pointer_cast<Unary>(node));
            break;
        case NodeKind::Variable:
            visit(std::static_pointer_cast<Variable>(node));
            break;
        case NodeKind::Text:
            visit(std::static_pointer_cast<Text>(node));
            break;
        case NodeKind::Number:
            visit(std::static_pointer_cast<Number>(node));
            break;
        case NodeKind::Boolean:
            visit(std::static_pointer_cast<Boolean>(node));
            break;
        default:
            // Handle unknown expression kind if necessary
            break;
        }
    }
};

} // basic
