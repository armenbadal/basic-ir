#include "semantic.hxx"

#include <algorithm>
#include <format>

namespace basic {

static Type::Ptr typeFromIdentifier(std::string_view identifier)
{
    if( identifier.ends_with('?') )
        return Types::boolean();
    if( identifier.ends_with('$') )
        return Types::text();
    return Types::real();
}

void SemanticModel::bind(NodeId node, SymbolId symbol)
{
    _symbols.insert_or_assign(node, symbol);
}

void SemanticModel::setType(NodeId node, Type::Ptr type)
{
    _types.insert_or_assign(node, std::move(type));
}

std::optional<SymbolId> SemanticModel::symbol(NodeId node) const
{
    if( auto it = _symbols.find(node); it != _symbols.end() )
        return it->second;

    return std::nullopt;
}

std::optional<Type::Ptr> SemanticModel::type(NodeId node) const
{
    if( auto it = _types.find(node); it != _types.end() )
        return it->second;

    return std::nullopt;
}

SemanticAnalyzer::SemanticAnalyzer(SymbolTable& symbols, SemanticModel& semantic, Diagnostics& diagnostics)
    : _symbols{symbols}, _semantic{semantic}, _diagnostics{diagnostics}
{
}

void SemanticAnalyzer::analyze(const Program::Ptr& program)
{
    if( program )
        visit(*program);
}

void SemanticAnalyzer::visit(Program& node)
{
    for( const auto& subroutine : node._subroutines ) {
        const auto id = _symbols.declareSubroutine(subroutine->_name, {});
        if( id == 0 ) {
            error(*subroutine, std::format("ենթածրագիր '{}' արդեն հայտարարված է", subroutine->_name));
            continue;
        }
        _semantic.bind(subroutine->id(), id);
    }

    for( const auto& subroutine : node._subroutines )
        visit(*subroutine);
}

void SemanticAnalyzer::visit(Subroutine& node)
{
    const auto subroutineId = _semantic.symbol(node.id());
    _symbols.openScope();

    std::vector<SymbolId> parameters;
    for( const auto& parameter : node._parameters ) {
        const auto id = _symbols.declareParameter(parameter->_name, typeFromIdentifier(parameter->_name));
        if( id == 0 ) {
            error(*parameter, std::format("պարամետր '{}' արդեն հայտարարված է", parameter->_name));
            continue;
        }
        parameters.push_back(id);
        _semantic.bind(parameter->id(), id);
        _semantic.setType(parameter->id(), typeFromIdentifier(parameter->_name));
    }

    if( subroutineId ) {
        auto& subroutine = static_cast<SubroutineSymbol&>(_symbols.symbol(*subroutineId));
        subroutine.setParameters(std::move(parameters));
    }

    visit(*node._body);
    _symbols.closeScope();
}

void SemanticAnalyzer::visit(Sequence& node)
{
    for( const auto& statement : node._items )
        visit(*statement);
}

void SemanticAnalyzer::visit(Dim& node)
{
    const auto sizeType = expressionType(node._size);
    requireType(*node._size, *sizeType, *Types::real(), "զանգվածի չափը");

    const auto type = Types::array(typeFromIdentifier(node._name));
    const auto id = _symbols.declareVariable(node._name, type);
    if( id == 0 ) {
        error(node, std::format("փոփոխական '{}' արդեն հայտարարված է", node._name));
        return;
    }
    _semantic.bind(node.id(), id);
    _semantic.setType(node.id(), type);
}

void SemanticAnalyzer::visit(Let& node)
{
    const auto valueType = expressionType(node._expr);
    auto id = _symbols.lookup(node._variable->_name);
    if( !id ) {
        const auto variableType = typeFromIdentifier(node._variable->_name);
        const auto newId = _symbols.declareVariable(node._variable->_name, variableType);
        if( newId != 0 )
            id = newId;
    }

    if( !id ) {
        error(*node._variable, std::format("փոփոխական '{}' արդեն հայտարարված է", node._variable->_name));
        return;
    }

    const auto& symbol = _symbols.symbol(*id);
    if( symbol.kind() != Symbol::Kind::Variable ) {
        error(*node._variable, std::format("'{}' փոփոխական չէ", node._variable->_name));
        return;
    }

    const auto& variable = static_cast<const VariableSymbol&>(symbol);
    _semantic.bind(node._variable->id(), *id);
    _semantic.setType(node._variable->id(), typeFromIdentifier(node._variable->_name));
    _semantic.setType(node.id(), typeFromIdentifier(node._variable->_name));
    requireType(node, *valueType, variable.type(), "վերագրման արժեքը");
}

void SemanticAnalyzer::visit(Input& node)
{
    visit(*node._variable);
}

void SemanticAnalyzer::visit(Print& node)
{
    expressionType(node._expr);
}

void SemanticAnalyzer::visit(If& node)
{
    for( const auto& branch : node._branches )
        visit(*branch);
    if( node._alternative )
        visit(*node._alternative);
}

void SemanticAnalyzer::visit(If::IfThen& node)
{
    const auto conditionType = expressionType(node._condition);
    requireType(*node._condition, *conditionType, *Types::boolean(), "IF-ի պայմանը");
    visit(*node._decision);
}

void SemanticAnalyzer::visit(While& node)
{
    const auto conditionType = expressionType(node._condition);
    requireType(*node._condition, *conditionType, *Types::boolean(), "WHILE-ի պայմանը");
    visit(*node._body);
}

void SemanticAnalyzer::visit(For& node)
{
    visit(*node._parameter);
    const auto parameterType = _semantic.type(node._parameter->id()).value_or(Types::real());
    requireType(*node._parameter, *parameterType, *Types::real(), "FOR-ի փոփոխականը");

    const auto beginType = expressionType(node._begin);
    const auto endType = expressionType(node._end);
    const auto stepType = expressionType(node._step);
    requireType(*node._begin, *beginType, *Types::real(), "FOR-ի սկզբնական արժեքը");
    requireType(*node._end, *endType, *Types::real(), "FOR-ի վերջնական արժեքը");
    requireType(*node._step, *stepType, *Types::real(), "FOR-ի քայլը");
    visit(*node._body);
}

void SemanticAnalyzer::visit(Call& node)
{
    visit(*node._subrCall);
    if( const auto id = _semantic.symbol(node._subrCall->id()) )
        _semantic.bind(node.id(), *id);
}

void SemanticAnalyzer::visit(Array& node)
{
    if( node._elements.empty() ) {
        _semantic.setType(node.id(), Types::array(Types::real()));
        return;
    }

    const auto elementType = expressionType(node._elements.front());
    for( std::size_t index = 1; index < node._elements.size(); ++index )
        requireType(*node._elements[index], *expressionType(node._elements[index]), *elementType, "զանգվածի տարրը");
    _semantic.setType(node.id(), Types::array(elementType));
}

void SemanticAnalyzer::visit(Apply& node)
{
    std::vector<Type::Ptr> arguments;
    arguments.reserve(node._arguments.size());
    for( const auto& argument : node._arguments )
        arguments.push_back(expressionType(argument));

    const auto id = _symbols.lookup(node._callee);
    if( !id ) {
        error(node, std::format("ենթածրագիր '{}' հայտարարված չէ", node._callee));
        _semantic.setType(node.id(), typeFromIdentifier(node._callee));
        return;
    }

    const auto& symbol = _symbols.symbol(*id);
    if( symbol.kind() != Symbol::Kind::Subroutine ) {
        error(node, std::format("'{}' ենթածրագիր չէ", node._callee));
        _semantic.setType(node.id(), typeFromIdentifier(node._callee));
        return;
    }

    const auto& subroutine = static_cast<const SubroutineSymbol&>(symbol);
    _semantic.bind(node.id(), *id);
    if( arguments.size() != subroutine.parameters().size() ) {
        error(node, std::format("'{}'-ի արգումենտների քանակը սխալ է", node._callee));
    }
    else {
        for( std::size_t index = 0; index < arguments.size(); ++index ) {
            const auto& parameter = static_cast<const VariableSymbol&>(_symbols.symbol(subroutine.parameters()[index]));
            requireType(node, *arguments[index], parameter.type(), "ենթածրագրի արգումենտը");
        }
    }
    _semantic.setType(node.id(), typeFromIdentifier(node._callee));
}

void SemanticAnalyzer::visit(Binary& node)
{
    const auto left = expressionType(node._left);
    const auto right = expressionType(node._right);
    if( node._operation == Operation::Index ) {
        requireType(*node._right, *right, *Types::real(), "զանգվածի ինդեքսը");
        if( left->kind() != Type::Kind::Array ) {
            error(*node._left, "ինդեքսավորումը պահանջում է զանգված");
            _semantic.setType(node.id(), Types::real());
            return;
        }
        const auto& array = static_cast<const ArrayType&>(*left);
        _semantic.setType(node.id(), array.elementTypePtr());
        return;
    }

    switch( node._operation ) {
        case Operation::Add:
        case Operation::Sub:
        case Operation::Mul:
        case Operation::Div:
        case Operation::Mod:
        case Operation::Quot:
        case Operation::Pow:
            requireType(node, *left, *Types::real(), "թվային գործողության ձախ օպերանդը");
            requireType(node, *right, *Types::real(), "թվային գործողության աջ օպերանդը");
            _semantic.setType(node.id(), Types::real());
            return;
        case Operation::And:
        case Operation::Or:
            requireType(node, *left, *Types::boolean(), "տրամաբանական գործողության ձախ օպերանդը");
            requireType(node, *right, *Types::boolean(), "տրամաբանական գործողության աջ օպերանդը");
            _semantic.setType(node.id(), Types::boolean());
            return;
        case Operation::Conc:
            requireType(node, *left, *Types::text(), "կցման ձախ օպերանդը");
            requireType(node, *right, *Types::text(), "կցման աջ օպերանդը");
            _semantic.setType(node.id(), Types::text());
            return;
        case Operation::Eq:
        case Operation::Ne:
        case Operation::Gt:
        case Operation::Ge:
        case Operation::Lt:
        case Operation::Le:
            if( !(*left == *right) )
                error(node, "համեմատության օպերանդների տիպերը տարբեր են");
            _semantic.setType(node.id(), Types::boolean());
            return;
        default:
            _semantic.setType(node.id(), Types::real());
    }
}

void SemanticAnalyzer::visit(Unary& node)
{
    const auto operand = expressionType(node._operand);
    if( node._operation == Operation::Not ) {
        requireType(node, *operand, *Types::boolean(), "NOT-ի օպերանդը");
        _semantic.setType(node.id(), Types::boolean());
        return;
    }
    requireType(node, *operand, *Types::real(), "ունար գործողության օպերանդը");
    _semantic.setType(node.id(), Types::real());
}

void SemanticAnalyzer::visit(Variable& node)
{
    const auto id = variableSymbol(node);
    if( !id ) {
        _semantic.setType(node.id(), typeFromIdentifier(node._name));
        return;
    }
    _semantic.setType(node.id(), typeFromIdentifier(node._name));
}

void SemanticAnalyzer::visit(Text& node)
{
    _semantic.setType(node.id(), Types::text());
}

void SemanticAnalyzer::visit(Number& node)
{
    _semantic.setType(node.id(), Types::real());
}

void SemanticAnalyzer::visit(Boolean& node)
{
    _semantic.setType(node.id(), Types::boolean());
}

Type::Ptr SemanticAnalyzer::expressionType(const Expression::Ptr& expression)
{
    visit(*expression);
    return _semantic.type(expression->id()).value_or(Types::real());
}

std::optional<SymbolId> SemanticAnalyzer::variableSymbol(const Variable& variable)
{
    const auto id = _symbols.lookup(variable._name);
    if( !id ) {
        error(variable, std::format("փոփոխական '{}' հայտարարված չէ", variable._name));
        return std::nullopt;
    }
    if( _symbols.symbol(*id).kind() != Symbol::Kind::Variable ) {
        error(variable, std::format("'{}' փոփոխական չէ", variable._name));
        return std::nullopt;
    }
    _semantic.bind(variable.id(), *id);
    return id;
}

void SemanticAnalyzer::requireType(const Node& node, const Type& actual, const Type& expected, std::string_view context)
{
    if( actual != expected )
        error(node, std::format("{} պետք է լինի {}, բայց {} է", context, expected.name(), actual.name()));
}

void SemanticAnalyzer::error(const Node& node, std::string_view message)
{
    _diagnostics.advance();
    _diagnostics.mark(node.line, message);
}

} // namespace basic
