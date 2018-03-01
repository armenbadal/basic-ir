
#include "Compiler.hxx"

namespace basic {
bool Compiler::compile(AstNode* node)
{
    compileAstNode(node);
    return true;
}

void Compiler::compileAstNode(AstNode* node)
{
    if (nullptr == node)
        return;

    void result = 0;
    switch (node->kind) {
        case NodeKind::Number:
            compileNumber(dynamic_cast<Number*>(node));
            break;
        case NodeKind::Text:
            compileText(dynamic_cast<Text*>(node));
            break;
        case NodeKind::Variable:
            compileVariable(dynamic_cast<Variable*>(node));
            break;
        case NodeKind::Unary:
            compileUnary(dynamic_cast<Unary*>(node));
            break;
        case NodeKind::Binary:
            compileBinary(dynamic_cast<Binary*>(node));
            break;
        case NodeKind::Apply:
            compileApply(dynamic_cast<Apply*>(node));
            break;
        case NodeKind::Sequence:
            compileSequence(dynamic_cast<Sequence*>(node));
            break;
        case NodeKind::Input:
            compileInput(dynamic_cast<Input*>(node));
            break;
        case NodeKind::Print:
            compilePrint(dynamic_cast<Print*>(node));
            break;
        case NodeKind::Let:
            compileLet(dynamic_cast<Let*>(node));
            break;
        case NodeKind::If:
            compileIf(dynamic_cast<If*>(node));
            break;
        case NodeKind::While:
            compileWhile(dynamic_cast<While*>(node));
            break;
        case NodeKind::For:
            compileFor(dynamic_cast<For*>(node));
            break;
        case NodeKind::Call:
            compileCall(dynamic_cast<Call*>(node));
            break;
        case NodeKind::Subroutine:
            compileSubroutine(dynamic_cast<Subroutine*>(node));
            break;
        case NodeKind::Program:
            compileProgram(dynamic_cast<Program*>(node));
            break;
        default:
            break;
    }
}
} // basic
