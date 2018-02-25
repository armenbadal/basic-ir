
#include "converter.hxx"

namespace basic {
bool Converter::convert(AstNode* node)
{
    convertAstNode(node);
    return true;
}

int Converter::convertAstNode(AstNode* node)
{
    if (nullptr == node)
        return 0;

    int result = 0;
    switch (node->kind) {
        case NodeKind::Number:
            result = convertNumber(dynamic_cast<Number*>(node));
            break;
        case NodeKind::Text:
            result = convertText(dynamic_cast<Text*>(node));
            break;
        case NodeKind::Variable:
            result = convertVariable(dynamic_cast<Variable*>(node));
            break;
        case NodeKind::Unary:
            result = convertUnary(dynamic_cast<Unary*>(node));
            break;
        case NodeKind::Binary:
            result = convertBinary(dynamic_cast<Binary*>(node));
            break;
        case NodeKind::Apply:
            result = convertApply(dynamic_cast<Apply*>(node));
            break;
        case NodeKind::Sequence:
            result = convertSequence(dynamic_cast<Sequence*>(node));
            break;
        case NodeKind::Input:
            result = convertInput(dynamic_cast<Input*>(node));
            break;
        case NodeKind::Print:
            result = convertPrint(dynamic_cast<Print*>(node));
            break;
        case NodeKind::Let:
            result = convertLet(dynamic_cast<Let*>(node));
            break;
        case NodeKind::If:
            result = convertIf(dynamic_cast<If*>(node));
            break;
        case NodeKind::While:
            result = convertWhile(dynamic_cast<While*>(node));
            break;
        case NodeKind::For:
            result = convertFor(dynamic_cast<For*>(node));
            break;
        case NodeKind::Call:
            result = convertCall(dynamic_cast<Call*>(node));
            break;
        case NodeKind::Subroutine:
            result = convertSubroutine(dynamic_cast<Subroutine*>(node));
            break;
        case NodeKind::Program:
            result = convertProgram(dynamic_cast<Program*>(node));
            break;
        default:
            break;
    }
    return result;
}
} // basic
