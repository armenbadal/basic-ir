
#include "astvisitor.hxx"

namespace basic {
void AstVisitor::visitAstNode(AstNode* node)
{
    switch (node->kind) {
        case NodeKind::Number:
            visitNumber(dynamic_cast<Number*>(node));
            break;
        case NodeKind::Text:
            visitText(dynamic_cast<Text*>(node));
            break;
        case NodeKind::Variable:
            visitVariable(dynamic_cast<Variable*>(node));
            break;
        case NodeKind::Unary:
            visitUnary(dynamic_cast<Unary*>(node));
            break;
        case NodeKind::Binary:
            visitBinary(dynamic_cast<Binary*>(node));
            break;
        case NodeKind::Apply:
            visitApply(dynamic_cast<Apply*>(node));
            break;
        case NodeKind::Sequence:
            visitSequence(dynamic_cast<Sequence*>(node));
            break;
        case NodeKind::Input:
            visitInput(dynamic_cast<Input*>(node));
            break;
        case NodeKind::Print:
            visitPrint(dynamic_cast<Print*>(node));
            break;
        case NodeKind::Let:
            visitLet(dynamic_cast<Let*>(node));
            break;
        case NodeKind::If:
            visitIf(dynamic_cast<If*>(node));
            break;
        case NodeKind::While:
            visitWhile(dynamic_cast<While*>(node));
            break;
        case NodeKind::For:
            visitFor(dynamic_cast<For*>(node));
            break;
        case NodeKind::Call:
            visitCall(dynamic_cast<Call*>(node));
            break;
        case NodeKind::Subroutine:
            visitSubroutine(dynamic_cast<Subroutine*>(node));
            break;
        case NodeKind::Program:
            visitProgram(dynamic_cast<Program*>(node));
            break;
        default:
            break;
    }
}
}
