
#include "astvisitor.hxx"

namespace basic {
void AstVisitor::visitAstNode(  AstNodePtr node )
{
    switch( node->kind ) {
        case NodeKind::Number:
            visitNumber(std::dynamic_pointer_cast<Number>(node));
            break;
        case NodeKind::Text:
            visitText(std::dynamic_pointer_cast<Text>(node));
            break;
        case NodeKind::Variable:
            visitVariable(std::dynamic_pointer_cast<Variable>(node));
            break;
        case NodeKind::Unary:
            visitUnary(std::dynamic_pointer_cast<Unary>(node));
            break;
        case NodeKind::Binary:
            visitBinary(std::dynamic_pointer_cast<Binary>(node));
            break;
        case NodeKind::Apply:
            visitApply(std::dynamic_pointer_cast<Apply>(node));
            break;
        case NodeKind::Sequence:
            visitSequence(std::dynamic_pointer_cast<Sequence>(node));
            break;
        case NodeKind::Input:
            visitInput(std::dynamic_pointer_cast<Input>(node));
            break;
        case NodeKind::Print:
            visitPrint(std::dynamic_pointer_cast<Print>(node));
            break;
        case NodeKind::Let:
            visitLet(std::dynamic_pointer_cast<Let>(node));
            break;
        case NodeKind::If:
            visitIf(std::dynamic_pointer_cast<If>(node));
            break;
        case NodeKind::While:
            visitWhile(std::dynamic_pointer_cast<While>(node));
            break;
        case NodeKind::For:
            visitFor(std::dynamic_pointer_cast<For>(node));
            break;
        case NodeKind::Call:
            visitCall(std::dynamic_pointer_cast<Call>(node));
            break;
        case NodeKind::Subroutine:
            visitSubroutine(std::dynamic_pointer_cast<Subroutine>(node));
            break;
        case NodeKind::Program:
            visitProgram(std::dynamic_pointer_cast<Program>(node));
            break;
        default:
            break;
    }
}
} // namespace basic
