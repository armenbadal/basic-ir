
#ifndef AST_VISITOR_HXX
#define AST_VISITOR_HXX

#include "ast.hxx"

#include <type_traits>

namespace basic {
template<typename P, typename F, typename S, typename E>
class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    
protected:
    virtual P visit(ProgramPtr node) = 0;
    virtual F visit(SubroutinePtr node) = 0;
    
    virtual S visit(StatementPtr node) = 0;
    virtual S visit(SequencePtr node) = 0;
    virtual S visit(LetPtr node) = 0;
    virtual S visit(InputPtr node) = 0;
    virtual S visit(PrintPtr node) = 0;
    virtual S visit(IfPtr node) = 0;
    virtual S visit(WhilePtr node) = 0;
    virtual S visit(ForPtr node) = 0;
    virtual S visit(CallPtr node) = 0;

    virtual E visit(ExpressionPtr node) = 0;
    virtual E visit(ApplyPtr node) = 0;
    virtual E visit(BinaryPtr node) = 0;
    virtual E visit(UnaryPtr node) = 0;
    virtual E visit(VariablePtr node) = 0;
    virtual E visit(TextPtr node) = 0;
    virtual E visit(NumberPtr node) = 0;
    virtual E visit(BooleanPtr node) = 0;

    S dispatch(StatementPtr node)
    {
        switch( node->kind ) {
            case NodeKind::Sequence:
                return visit(std::dynamic_pointer_cast<Sequence>(node));
            case NodeKind::Input:
                return visit(std::dynamic_pointer_cast<Input>(node));
            case NodeKind::Print:
                return visit(std::dynamic_pointer_cast<Print>(node));
            case NodeKind::Let:
                return visit(std::dynamic_pointer_cast<Let>(node));
            case NodeKind::If:
                return visit(std::dynamic_pointer_cast<If>(node));
            case NodeKind::While:
                return visit(std::dynamic_pointer_cast<While>(node));
            case NodeKind::For:
                return visit(std::dynamic_pointer_cast<For>(node));
            case NodeKind::Call:
                return visit(std::dynamic_pointer_cast<Call>(node));
            default:
                break;
        }

        if constexpr( !std::is_same_v<S,void> )
            return {};
    }

    E dispatch(ExpressionPtr node)
    {
        switch( node->kind ) {
            case NodeKind::Boolean:
                return visit(std::dynamic_pointer_cast<Boolean>(node));
            case NodeKind::Number:
                return visit(std::dynamic_pointer_cast<Number>(node));
            case NodeKind::Text:
                return visit(std::dynamic_pointer_cast<Text>(node));
            case NodeKind::Variable:
                return visit(std::dynamic_pointer_cast<Variable>(node));
            case NodeKind::Unary:
                return visit(std::dynamic_pointer_cast<Unary>(node));
            case NodeKind::Binary:
                return visit(std::dynamic_pointer_cast<Binary>(node));
            case NodeKind::Apply:
                return visit(std::dynamic_pointer_cast<Apply>(node));
            default:
                break;
        }

        if constexpr( !std::is_same_v<E,void> )
            return {};
    }
};
} // basic

#endif // AST_VISITOR_HXX
