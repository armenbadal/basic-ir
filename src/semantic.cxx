#include "semantic.hxx"

#include <format>
#include <set>
#include <utility>

namespace basic {

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
    : _symbols{symbols}
    , _semantic{semantic}
    , _diagnostics{diagnostics}
{
}

void SemanticAnalyzer::analyze(const Program::Ptr& program)
{
    if( program )
        visit(*program);
}

void SemanticAnalyzer::visit(Program& node)
{
    declareSubroutines(node);

    for( const auto& subroutine : node._subroutines )
        visit(*subroutine);
}

void SemanticAnalyzer::declareSubroutines(const Program& node)
{
    for( const auto& subroutine : node._subroutines ) {
        auto ps = parameterTypes(*subroutine);
        auto r = Types::fromKeyword(subroutine->_returnType);
        const auto id = _symbols.declareSubroutine(subroutine->_name, ps, r);
        if( id == UnknownSymbol ) {
            error(*subroutine, "'{}' ենթածրագիրն արդեն հայտարարված է", subroutine->_name);
            continue;
        }

        _semantic.bind(subroutine->id(), id);
    }
}

Type::Ptr SemanticAnalyzer::parameterType(const Dim& parameter)
{
    const auto elementType = Types::fromKeyword(parameter._type);
    return parameter._isArray ? Types::array(elementType) : elementType;
}

std::vector<Type::Ptr> SemanticAnalyzer::parameterTypes(const Subroutine& subroutine)
{
    std::vector<Type::Ptr> types;
    std::set<std::string> seen;
    for( const auto& parameter : subroutine._parameters ) {
        if( !seen.insert(parameter->_name).second ) {
            error(*parameter, "'{}' պարամետրն արդեն հայտարարված է", parameter->_name);
            continue;
        }
        types.push_back(parameterType(*parameter));
    }
    return types;
}

void SemanticAnalyzer::visit(Subroutine& node)
{
    _symbols.openScope();

    // պարամետրերը՝ հայտարարված տիպերով
    for( const auto& parameter : node._parameters ) {
        const auto type = parameterType(*parameter);
        const auto id = _symbols.declareParameter(parameter->_name, type);
        if( id == UnknownSymbol )
            continue;
        _semantic.bind(parameter->id(), id);
        _semantic.setType(parameter->id(), type);
    }

    if( node._returnType != "nothing" ) {
        if( const auto returnType = Types::fromKeyword(node._returnType) ) {
            const auto id = _symbols.declareVariable(node._name, returnType, VariableSymbol::Storage::ReturnValue);
            if( id == UnknownSymbol )
                error(node, "'{}' փոփոխականն արդեն հայտարարված է", node._name);
        }
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
    if( node._size ) {
        const auto sizeType = expressionType(node._size);
        requireType(*node._size, *sizeType, *Types::real(), "զանգվածի չափը");
    }

    const auto elementType = Types::fromKeyword(node._type);
    const auto type = node._isArray ? Types::array(elementType) : elementType;
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

    // LET-ը երբեք չի հայտարարում. ձախ մասը պետք է գոյություն ունեցող
    // փոփոխական լինի, հակառակ դեպքում variableSymbol-ը սխալ է հայտնում
    const auto id = variableSymbol(*node._variable);
    if( !id )
        return;

    const auto& variable = static_cast<const VariableSymbol&>(_symbols.symbol(*id));
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
    // FOR-ի պարամետրը իմպլիցիտ հայտարարվում է որպես REAL,
    // եթե նույն անունով փոփոխական գոյություն չունի
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
        {"STR", {1, Types::text()}},
        {"MID", {3, Types::text()}},
        {"SQR", {1, Types::real()}},
        {"SIN", {1, Types::real()}},
        {"COS", {1, Types::real()}},
        {"ABS", {1, Types::real()}},
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

    // փնտրում ենք կանչվող ենթածրագիրը՝ անտեսելով նույն անունով
    // փոփոխականները (որ չխանգարի վերադարձի փոփոխականը ռեկուրսիայի ժամանակ)
    const auto id = _symbols.lookupSubroutine(node._callee);
    if( !id ) {
        error(node, "'{}' անունով ենթածրագիր սահմանված չէ", node._callee);
        _semantic.setType(node.id(), Types::real());
        return;
    }

    const auto& subroutine = static_cast<const SubroutineSymbol&>(_symbols.symbol(*id));
    _semantic.bind(node.id(), *id);

    // ստուգում ենք արգումենտների քանակի ու տիպերի համապատասխանությունը
    // ենթածրագրի ստորագրության հետ
    const auto& parameterTypes = subroutine.parameterTypes();
    if( node._arguments.size() != parameterTypes.size() ) {
        error(node, "'{}'-ի արգումենտների քանակը սխալ է", node._callee);
    }
    else {
        for( std::size_t index = 0; index < node._arguments.size(); ++index ) {
            const auto argumentType = _semantic.type(node._arguments[index]->id()).value_or(Types::real());
            requireType(*node._arguments[index], *argumentType, *parameterTypes[index],
                std::format("'{}'-ի {} արգումենտը", node._callee, index + 1));
        }
    }

    // վերադարձի տիպը որոշվում է միայն հայտարարված AS-ով. պրոցեդուրան
    // (NOTHING տիպով) արժեք չի վերադարձնում, և դրա կանչը արտահայտության
    // մեջ կբռնվի տիպերի անհամապատասխանությամբ
    if( const auto returnType = subroutine.returnType() )
        _semantic.setType(node.id(), *returnType);
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
