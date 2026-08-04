#include "semantic.hxx"

#include <catch2/catch_test_macros.hpp>

using namespace basic;

TEST_CASE("SemanticModel stores node annotations", "[semantic]")
{
    SemanticModel model;

    CHECK(model.symbol(1) == std::nullopt);
    CHECK(model.type(1) == std::nullopt);

    model.bind(1, 42);
    model.setType(1, Types::text());

    CHECK(model.symbol(1) == 42);
    REQUIRE(model.type(1).has_value());
    CHECK(**model.type(1) == *Types::text());

    model.bind(1, 43);
    model.setType(1, Types::real());

    CHECK(model.symbol(1) == 43);
    REQUIRE(model.type(1).has_value());
    CHECK(**model.type(1) == *Types::real());
}

TEST_CASE("Semantic analysis annotates declarations and calls", "[semantic]")
{
    auto parameter = node<Dim>("message", nullptr, "REAL", false, 1);
    auto message = node<Variable>("message", 1);
    auto echo = node<Subroutine>("Echo", std::vector<Dim::Ptr>{parameter}, "nothing",
        node<Sequence>(std::vector<Statement::Ptr>{node<Let>(message, node<Number>(1, 2), 2)}, 2), 1);

    auto buffer = node<Dim>("buffer", node<Number>(10, 1), "REAL", true, 1);
    auto count = node<Variable>("count", 4);
    auto countDecl = node<Dim>("count", nullptr, "REAL", false, 3);
    auto call = node<Call>("Echo", std::vector<Expression::Ptr>{count}, 5);
    auto main = node<Subroutine>("Main", std::vector<Dim::Ptr>{}, "nothing",
        node<Sequence>(std::vector<Statement::Ptr>{buffer, countDecl, node<Let>(count, node<Number>(1, 4), 4), call}, 4), 3);
    auto program = node<Program>(std::vector<Subroutine::Ptr>{echo, main}, 1);

    SymbolTable symbols;
    SemanticModel model;
    Diagnostics diagnostics;
    SemanticAnalyzer analyzer{symbols, model, diagnostics};
    analyzer.analyze(program);

    CHECK(diagnostics.count() == 0);
    REQUIRE(model.type(count->id()).has_value());
    CHECK(**model.type(count->id()) == *Types::real());
    CHECK(model.symbol(call->id()).has_value());
}

TEST_CASE("Semantic analysis respects declared parameter types", "[semantic]")
{
    auto parameter = node<Dim>("x", nullptr, "TEXT", false, 1);
    auto x = node<Variable>("x", 2);
    // DIM y AS REAL ... LET y = x + 1. «+»-ն սպասում է REAL օպերանդներ, իսկ x-ը TEXT է
    auto body = node<Sequence>(
        std::vector<Statement::Ptr>{
            node<Dim>("y", nullptr, "REAL", false, 2),
            node<Let>(node<Variable>("y", 3),
                node<Binary>(Operation::Add, x, node<Number>(1, 3), 3), 3)},
        2);
    auto sub = node<Subroutine>("f", std::vector<Dim::Ptr>{parameter}, "nothing", body, 1);
    auto program = node<Program>(std::vector<Subroutine::Ptr>{sub}, 1);

    SymbolTable symbols;
    SemanticModel model;
    Diagnostics diagnostics;
    SemanticAnalyzer analyzer{symbols, model, diagnostics};
    analyzer.analyze(program);

    CHECK(diagnostics.count() == 1);
}

TEST_CASE("Semantic analysis checks assignments to the declared return type", "[semantic]")
{
    // SUB f AS TEXT ... LET f = 1. Հայտարարված տիպը TEXT է, իսկ 1-ը՝ REAL
    auto f = node<Variable>("f", 2);
    auto body = node<Sequence>(
        std::vector<Statement::Ptr>{node<Let>(f, node<Number>(1, 2), 2)}, 2);
    auto sub = node<Subroutine>("f", std::vector<Dim::Ptr>{}, "TEXT", body, 1);
    auto program = node<Program>(std::vector<Subroutine::Ptr>{sub}, 1);

    SymbolTable symbols;
    SemanticModel model;
    Diagnostics diagnostics;
    SemanticAnalyzer analyzer{symbols, model, diagnostics};
    analyzer.analyze(program);

    CHECK(diagnostics.count() == 1);
}

TEST_CASE("Semantic analysis resolves declared return type at call sites", "[semantic]")
{
    // SUB f(x AS REAL) AS BOOL ... LET f = x > 0
    auto parameter = node<Dim>("x", nullptr, "REAL", false, 1);
    auto x = node<Variable>("x", 2);
    auto f = node<Variable>("f", 2);
    auto body = node<Sequence>(
        std::vector<Statement::Ptr>{
            node<Let>(f, node<Binary>(Operation::Gt, x, node<Number>(0, 2), 2), 2)},
        2);
    auto func = node<Subroutine>("f", std::vector<Dim::Ptr>{parameter}, "BOOL", body, 1);

    // SUB Main ... LET y = f(1)
    auto apply = node<Apply>("f", std::vector<Expression::Ptr>{node<Number>(1, 4)}, 4);
    auto y = node<Variable>("y", 4);
    auto yDecl = node<Dim>("y", nullptr, "BOOL", false, 3);
    auto main = node<Subroutine>("Main", std::vector<Dim::Ptr>{}, "nothing",
        node<Sequence>(std::vector<Statement::Ptr>{yDecl, node<Let>(y, apply, 4)}, 4), 3);
    auto program = node<Program>(std::vector<Subroutine::Ptr>{func, main}, 1);

    SymbolTable symbols;
    SemanticModel model;
    Diagnostics diagnostics;
    SemanticAnalyzer analyzer{symbols, model, diagnostics};
    analyzer.analyze(program);

    CHECK(diagnostics.count() == 0);
    REQUIRE(model.type(apply->id()).has_value());
    CHECK(**model.type(apply->id()) == *Types::boolean());
}

TEST_CASE("Semantic analysis checks argument types against parameters", "[semantic]")
{
    // SUB g(x AS REAL) ... CALL g("text") — արգումենտը TEXT է, պարամետրը՝ REAL
    auto parameter = node<Dim>("x", nullptr, "REAL", false, 1);
    auto func = node<Subroutine>("g", std::vector<Dim::Ptr>{parameter}, "nothing",
        node<Sequence>(std::vector<Statement::Ptr>{}, 2), 1);
    auto call = node<Call>("g", std::vector<Expression::Ptr>{node<Text>("text", 3)}, 3);
    auto main = node<Subroutine>("Main", std::vector<Dim::Ptr>{}, "nothing",
        node<Sequence>(std::vector<Statement::Ptr>{call}, 3), 2);
    auto program = node<Program>(std::vector<Subroutine::Ptr>{func, main}, 1);

    SymbolTable symbols;
    SemanticModel model;
    Diagnostics diagnostics;
    SemanticAnalyzer analyzer{symbols, model, diagnostics};
    analyzer.analyze(program);

    CHECK(diagnostics.count() == 1);
}
