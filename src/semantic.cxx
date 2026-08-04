#include "semantic.hxx"

#include <algorithm>
#include <format>
#include <tuple>
#include <utility>

namespace basic {

static Type::Ptr typeFromKeyword(std::string_view keyword)
{
    if( keyword == "TEXT" )
        return Types::text();
    if( keyword == "BOOL" )
        return Types::boolean();
    if( keyword == "REAL" )
        return Types::real();
    return nullptr;
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
    // նախ հայտարարում ենք բոլոր ենթածրագրերը, որպեսզի կանչի սխալներ չլինեն
    for( const auto& subroutine : node._subroutines ) {
        const auto id = _symbols.declareSubroutine(subroutine->_name, {});
        if( id == UnknownSymbol ) {
            error(*subroutine, "'{}' ենթածրագիրն արդեն հայտարարված է", subroutine->_name);
            continue;
        }
        _semantic.bind(subroutine->id(), id);
    }

    // առաջին փուլ. որոշել բոլոր ենթածրագրերի պարամետրերի ցուցակը
    // և վերադարձրած արժեքի տիպը, որպեսզի կանչի ստուգումը հաշվի
    // առնի հետո հայտարարված ենթածրագրերը
    for( const auto& subroutine : node._subroutines ) {
        const auto id = _semantic.symbol(subroutine->id());
        if( id ) {
            auto& subr = static_cast<SubroutineSymbol&>(_symbols.symbol(*id));
            subr.setParameters(declaredParameters(*subroutine));
            subr.setReturnType(declaredReturnType(*subroutine));
        }
    }

    // երկրորդ փուլ. ստուգել բոլոր ենթածրագրերի մարմինները
    for( const auto& subroutine : node._subroutines )
        visit(*subroutine);
}

std::vector<SymbolId> SemanticAnalyzer::declaredParameters(Subroutine& subroutine)
{
    _symbols.openScope();

    std::vector<SymbolId> parameters;
    for( const auto& parameter : subroutine._parameters ) {
        const auto type = parameterType(*parameter);
        const auto id = _symbols.declareParameter(parameter->_name, type);
        if( id == UnknownSymbol ) {
            error(*parameter, "'{}' պարամետրն արդեն հայտարարված է", parameter->_name);
            continue;
        }
        parameters.push_back(id);
        _semantic.bind(parameter->id(), id);
        _semantic.setType(parameter->id(), type);
    }

    _symbols.closeScope();
    return parameters;
}

Type::Ptr SemanticAnalyzer::parameterType(const Dim& parameter)
{
    const auto elementType = typeFromKeyword(parameter._type);
    return parameter._size ? Types::array(elementType) : elementType;
}

Type::Ptr SemanticAnalyzer::declaredReturnType(const Subroutine& subroutine)
{
    if( subroutine._returnType == "empty" )
        return Types::real();
    return typeFromKeyword(subroutine._returnType);
}

void SemanticAnalyzer::visit(Subroutine& node)
{
    _symbols.openScope();
    for( const auto& parameter : node._parameters )
        _symbols.declareParameter(parameter->_name, parameterType(*parameter));

    _currentSubroutine = node._name;
    _currentReturnType = node._returnType == "empty"
        ? std::nullopt
        : std::optional<Type::Ptr>{typeFromKeyword(node._returnType)};
    visit(*node._body);
    _currentSubroutine.clear();
    _currentReturnType.reset();

    _symbols.closeScope();
}

void SemanticAnalyzer::visit(Sequence& node)
{
    for( const auto& statement : node._items )
        visit(*statement);
}

void SemanticAnalyzer::visit(Dim& node)
{
    if( node._size ) {
        const auto sizeType = expressionType(node._size);
        requireType(*node._size, *sizeType, *Types::real(), "զանգվածի չափը");
    }

    const auto elementType = typeFromKeyword(node._type);
    const auto type = node._size ? Types::array(elementType) : elementType;
    const auto id = _symbols.declareVariable(node._name, type);
    if( id == UnknownSymbol ) {
        error(node, "'{}' փոփոխականն արդեն հայտարարված է", node._name);
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
        const auto newId = _symbols.declareVariable(node._variable->_name, valueType);
        if( newId != UnknownSymbol )
            id = newId;
    }

    if( !id ) {
        error(*node._variable, "'{}' փոփոխականն արդեն հայտարարված է", node._variable->_name);
        return;
    }

    const auto& symbol = _symbols.symbol(*id);
    if( symbol.kind() != Symbol::Kind::Variable ) {
        // ենթածրագիրն իր անունով է վերադարձնում արժեքը
        if( symbol.kind() == Symbol::Kind::Subroutine && node._variable->_name == _currentSubroutine ) {
            auto& subroutine = static_cast<SubroutineSymbol&>(_symbols.symbol(*id));
            // եթե ենթածրագիրն ունի հայտարարված վերադարձի տիպ, ապա
            // անվանը վերագրումը պետք է համապատասխանի այդ տիպին
            if( _currentReturnType ) {
                requireType(node, *valueType, **_currentReturnType,
                            std::format("'{}' ենթածրագրի վերադարձրած արժեքը", _currentSubroutine));
                _semantic.bind(node._variable->id(), *id);
                _semantic.setType(node._variable->id(), *_currentReturnType);
                _semantic.setType(node.id(), *_currentReturnType);
                return;
            }
            subroutine.setReturnType(valueType);
            _semantic.bind(node._variable->id(), *id);
            _semantic.setType(node._variable->id(), valueType);
            _semantic.setType(node.id(), valueType);
            return;
        }
        error(*node._variable, "'{}' փոփոխական չէ", node._variable->_name);
        return;
    }

    const auto& variable = static_cast<const VariableSymbol&>(symbol);
    _semantic.bind(node._variable->id(), *id);
    _semantic.setType(node._variable->id(), variable.typePtr());
    _semantic.setType(node.id(), variable.typePtr());
    requireType(node, *valueType, variable.type(), "վերագրման արժեքը");
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
    requireCondition(*node._condition, *conditionType, "IF-ի պայմանը");
    visit(*node._decision);
}

void SemanticAnalyzer::visit(While& node)
{
    const auto conditionType = expressionType(node._condition);
    requireCondition(*node._condition, *conditionType, "WHILE-ի պայմանը");
    visit(*node._body);
}

void SemanticAnalyzer::visit(For& node)
{
    if( !_symbols.lookup(node._parameter->_name) )
        _symbols.declareVariable(node._parameter->_name, Types::real());

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
    struct Builtin {
        std::size_t arity;
        Type::Ptr type;
    };
    static const std::unordered_map<std::string, Builtin> builtins{
        { "STR", { 1, Types::text() } },
        { "MID", { 3, Types::text() } },
        { "SQR", { 1, Types::real() } },
        { "SIN", { 1, Types::real() } },
        { "COS", { 1, Types::real() } },
        { "ABS", { 1, Types::real() } },
    };

    if( auto it = builtins.find(node._callee); it != builtins.end() ) {
        for( const auto& argument : node._arguments )
            expressionType(argument);
        if( node._arguments.size() != it->second.arity )
            error(node, "'{}'-ի արգումենտների քանակը սխալ է", node._callee);
        _semantic.setType(node.id(), it->second.type);
        return;
    }

    // հավաքում ենք կանչի արգումենտների տիպերը
    for( const auto& argument : node._arguments )
        expressionType(argument);

    // փնտրում ենք կանչվող ենթածրագիրը
    const auto id = _symbols.lookup(node._callee);
    if( !id ) {
        error(node, "'{}' անունով ենթածրագիր սահմանված չէ", node._callee);
        _semantic.setType(node.id(), Types::real());
        return;
    }

    // ստուգում ենք, որ գտած օբյեկտը ենթածրագիր է
    const auto& symbol = _symbols.symbol(*id);
    if( symbol.kind() != Symbol::Kind::Subroutine ) {
        error(node, "'{}'-ը ենթածրագիր չէ", node._callee);
        _semantic.setType(node.id(), Types::real());
        return;
    }

    // ստուգում ենք արգումենտների քանակի համապատասխանությունը
    // ենթածրագրի սահմանման հետ
    const auto& subroutine = static_cast<const SubroutineSymbol&>(symbol);
    _semantic.bind(node.id(), *id);
    if( node._arguments.size() != subroutine.parameters().size() ) {
        error(node, "'{}'-ի արգումենտների քանակը սխալ է", node._callee);
    } else {
        // ստուգում ենք յուրաքանչյուր արգումենտի տիպը պարամետրի տիպի հետ
        for( std::size_t index = 0; index < node._arguments.size(); ++index ) {
            const auto& parameter = static_cast<const VariableSymbol&>(
                _symbols.symbol(subroutine.parameters()[index]));
            const auto argumentType = _semantic.type(node._arguments[index]->id()).value_or(Types::real());
            requireType(*node._arguments[index], *argumentType, parameter.type(),
                        std::format("'{}'-ի {} արգումենտը", node._callee, index + 1));
        }
    }

    // ենթածրագրի վերադարձի տիպը հայտարարումից է
    _semantic.setType(node.id(), subroutine.returnType());
}

void SemanticAnalyzer::visit(Binary& node)
{
    // երկտեղանի գործողության ձախ ու աջ օպերանդների տիպերը
    const auto left = expressionType(node._left);
    const auto right = expressionType(node._right);

    // եթե սա ինդեքսավորման գործողություն է, ...
    if( node._operation == Operation::Index ) {
        // ... ապա սպասում ենք, որ աջ օպերանդը թիվ է, ...
        requireType(*node._right, *right, *Types::real(), "զանգվածի ինդեքսը");
        // ... իսկ ձախ կոզմում զանգված է
        if( left->kind() != Type::Kind::Array ) {
            error(*node._left, "[] գործողության ձախ կոզմում զանգված չէ");
            _semantic.setType(node.id(), Types::real());
            return;
        }
        const auto& array = static_cast<const ArrayType&>(*left);
        _semantic.setType(node.id(), array.elementTypePtr());
        return;
    }

    // մյուս երկտեղանի գործողությոնների երկու օպերանդներն էլ պետք 
    // է ունենան նույն տիպը, իսկ արդյունքի տիպը որոծշվում է 
    // գործողության տեսակով
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
            _semantic.setType(node.id(), Types::boolean());
            return;
        case Operation::Conc:
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
    expressionType(node._operand);
    if( node._operation == Operation::Not ) {
        _semantic.setType(node.id(), Types::boolean());
        return;
    }
    _semantic.setType(node.id(), Types::real());
}

void SemanticAnalyzer::visit(Variable& node)
{
    const auto id = variableSymbol(node);
    if( !id ) {
        _semantic.setType(node.id(), Types::real());
        return;
    }
    const auto& variable = static_cast<const VariableSymbol&>(_symbols.symbol(*id));
    _semantic.setType(node.id(), variable.typePtr());
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
        error(variable, "'{}' փոփոխականը հայտարարված չէ", variable._name);
        return std::nullopt;
    }

    if( _symbols.symbol(*id).kind() != Symbol::Kind::Variable ) {
        error(variable, "'{}'-ը փոփոխական չէ", variable._name);
        return std::nullopt;
    }

    _semantic.bind(variable.id(), *id);
    return id;
}

void SemanticAnalyzer::requireType(const Node& node, const Type& actual, const Type& expected, std::string_view context)
{
    if( actual != expected )
        error(node, "{} պետք է լինի {}, բայց {} է", context, expected.name(), actual.name());
}

void SemanticAnalyzer::requireCondition(const Node& node, const Type& actual, std::string_view context)
{
    if( actual != *Types::boolean() && actual != *Types::real() )
        error(node, "{} պետք է լինի BOOLEAN կամ REAL, բայց {} է", context, actual.name());
}

} // namespace basic
