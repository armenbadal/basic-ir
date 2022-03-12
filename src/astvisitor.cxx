
#include "astvisitor.hxx"

namespace basic {
void AstVisitor::visit(NodePtr node)
{
    if( nullptr == node )
        return;

    switch( node->kind ) {
        case NodeKind::Boolean:
            visit(std::dynamic_pointer_cast<Boolean>(node));
            break;
        case NodeKind::Number:
            visit(std::dynamic_pointer_cast<Number>(node));
            break;
        case NodeKind::Text:
            visit(std::dynamic_pointer_cast<Text>(node));
            break;
        case NodeKind::Variable:
            visit(std::dynamic_pointer_cast<Variable>(node));
            break;
        case NodeKind::Unary:
            visit(std::dynamic_pointer_cast<Unary>(node));
            break;
        case NodeKind::Binary:
            visit(std::dynamic_pointer_cast<Binary>(node));
            break;
        case NodeKind::Apply:
            visit(std::dynamic_pointer_cast<Apply>(node));
            break;
        case NodeKind::Sequence:
            visit(std::dynamic_pointer_cast<Sequence>(node));
            break;
        case NodeKind::Input:
            visit(std::dynamic_pointer_cast<Input>(node));
            break;
        case NodeKind::Print:
            visit(std::dynamic_pointer_cast<Print>(node));
            break;
        case NodeKind::Let:
            visit(std::dynamic_pointer_cast<Let>(node));
            break;
        case NodeKind::If:
            visit(std::dynamic_pointer_cast<If>(node));
            break;
        case NodeKind::While:
            visit(std::dynamic_pointer_cast<While>(node));
            break;
        case NodeKind::For:
            visit(std::dynamic_pointer_cast<For>(node));
            break;
        case NodeKind::Call:
            visit(std::dynamic_pointer_cast<Call>(node));
            break;
        case NodeKind::Subroutine:
            visit(std::dynamic_pointer_cast<Subroutine>(node));
            break;
        case NodeKind::Program:
            visit(std::dynamic_pointer_cast<Program>(node));
            break;
        default:
            break;
    }
}

} // namespace basic
