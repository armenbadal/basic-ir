
#include "converter.hxx"

namespace basic {
bool Converter::convert(AstNode* node, std::ostream& ooo)
{
    convertAstNode(node, ooo);
    return true;
}

int Converter::convertAstNode(AstNode* node, std::ostream& ooo)
{
    int result = 0;
    switch (node->kind) {
        case NodeKind::Number:
            result = convertNumber(dynamic_cast<Number*>(node), ooo);
            break;
        case NodeKind::Text:
            result = convertText(dynamic_cast<Text*>(node), ooo);
            break;
        case NodeKind::Variable:
            result = convertVariable(dynamic_cast<Variable*>(node), ooo);
            break;
        case NodeKind::Unary:
            result = convertUnary(dynamic_cast<Unary*>(node), ooo);
            break;
        case NodeKind::Binary:
            result = convertBinary(dynamic_cast<Binary*>(node), ooo);
            break;
        case NodeKind::Apply:
            result = convertApply(dynamic_cast<Apply*>(node), ooo);
            break;
        case NodeKind::Sequence:
            result = convertSequence(dynamic_cast<Sequence*>(node), ooo);
            break;
        case NodeKind::Input:
            result = convertInput(dynamic_cast<Input*>(node), ooo);
            break;
        case NodeKind::Print:
            result = convertPrint(dynamic_cast<Print*>(node), ooo);
            break;
        case NodeKind::Let:
            result = convertLet(dynamic_cast<Let*>(node), ooo);
            break;
        case NodeKind::If:
            result = convertIf(dynamic_cast<If*>(node), ooo);
            break;
        case NodeKind::While:
            result = convertWhile(dynamic_cast<While*>(node), ooo);
            break;
        case NodeKind::For:
            result = convertFor(dynamic_cast<For*>(node), ooo);
            break;
        case NodeKind::Call:
            result = convertCall(dynamic_cast<Call*>(node), ooo);
            break;
        case NodeKind::Subroutine:
            result = convertSubroutine(dynamic_cast<Subroutine*>(node), ooo);
            break;
        case NodeKind::Program:
            result = convertProgram(dynamic_cast<Program*>(node), ooo);
            break;
        default: {
        }
    }
    return result;
}
} // basic
