#include "semantic.hxx"

#include <catch2/catch_test_macros.hpp>

using namespace basic;

TEST_CASE("SemanticModel stores node annotations", "[semantic]")
{
    SemanticModel model;

    CHECK(model.symbol(1) == std::nullopt);
    CHECK(model.type(1) == std::nullopt);

    model.bind(1, 42);
    model.setType(1, Types::integer());

    CHECK(model.symbol(1) == 42);
    REQUIRE(model.type(1).has_value());
    CHECK(**model.type(1) == *Types::integer());

    model.bind(1, 43);
    model.setType(1, Types::real());

    CHECK(model.symbol(1) == 43);
    REQUIRE(model.type(1).has_value());
    CHECK(**model.type(1) == *Types::real());
}
