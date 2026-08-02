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
    auto parameter = node<Variable>("message$", 1);
    auto echo = node<Subroutine>("Echo", std::vector<Variable::Ptr>{parameter}, node<Sequence>(std::vector<Statement::Ptr>{node<Print>(parameter, 2)}, 2), 1);

    auto value = node<Variable>("value$", 4);
    auto call = node<Call>("Echo", std::vector<Expression::Ptr>{value}, 5);
    auto main = node<Subroutine>("Main", std::vector<Variable::Ptr>{}, node<Sequence>(std::vector<Statement::Ptr>{node<Let>(value, node<Text>("hello", 4), 4), call}, 4), 3);
    auto program = node<Program>(std::vector<Subroutine::Ptr>{echo, main}, 1);

    SymbolTable symbols;
    SemanticModel model;
    Diagnostics diagnostics;
    SemanticAnalyzer analyzer{symbols, model, diagnostics};
    analyzer.analyze(program);

    CHECK(diagnostics.count() == 0);
    REQUIRE(model.type(value->id()).has_value());
    CHECK(**model.type(value->id()) == *Types::text());
    CHECK(model.symbol(call->id()).has_value());
}
