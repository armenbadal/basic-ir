#include "types.hxx"
#include "symbol.hxx"

#include <catch2/catch_test_macros.hpp>

using namespace basic;

TEST_CASE("ScalarType names and equality", "[types]")
{
    auto realType1 = Types::real();
    auto realType2 = Types::real();
    auto boolType = Types::boolean();
    auto textType = Types::text();

    CHECK(realType1->name() == "REAL");
    CHECK(boolType->name() == "BOOLEAN");
    CHECK(textType->name() == "TEXT");

    CHECK(realType1->kind() == Type::Kind::Scalar);
    CHECK(boolType->kind() == Type::Kind::Scalar);
    CHECK(textType->kind() == Type::Kind::Scalar);

    CHECK(*realType1 == *realType2);
    CHECK(realType1 == realType2); // singleton check
    CHECK_FALSE(*realType1 == *boolType);
    CHECK_FALSE(*realType1 == *textType);
}

TEST_CASE("ArrayType names, equality, and elementType", "[types]")
{
    auto realArray1 = Types::array(Types::real());
    auto realArray2 = Types::array(Types::real());
    auto textArray = Types::array(Types::text());

    CHECK(realArray1->name() == "ARRAY");
    CHECK(realArray1->kind() == Type::Kind::Array);

    CHECK(*realArray1 == *realArray2);
    CHECK_FALSE(*realArray1 == *textArray);

    auto arrayTypePtr = std::dynamic_pointer_cast<const ArrayType>(realArray1);
    REQUIRE(arrayTypePtr != nullptr);
    CHECK(arrayTypePtr->elementType() == *Types::real());
}

TEST_CASE("VariableSymbol functionality", "[symbol]")
{
    VariableSymbol varSym{1, "x", Types::real()};
    CHECK(varSym.id() == 1);
    CHECK(varSym.name() == "x");
    CHECK(varSym.kind() == Symbol::Kind::Variable);
    CHECK(varSym.type() == *Types::real());
}

TEST_CASE("SubroutineSymbol functionality", "[symbol]")
{
    SubroutineSymbol subSym{2, "foo", {10, 11}};
    CHECK(subSym.id() == 2);
    CHECK(subSym.name() == "foo");
    CHECK(subSym.kind() == Symbol::Kind::Subroutine);
    CHECK(subSym.parameters() == std::vector<SymbolId>{10, 11});
}

TEST_CASE("SymbolTable declarations and lookup", "[symbol]")
{
    SymbolTable table;

    SymbolId x = table.declareVariable("x", Types::real());
    SymbolId p = table.declareParameter("p", Types::text());
    SymbolId foo = table.declareSubroutine("foo", {x, p});

    CHECK(table.lookup("x") == x);
    CHECK(table.lookup("p") == p);
    CHECK(table.lookup("foo") == foo);
    CHECK(table.lookup("missing") == std::nullopt);
    CHECK(table.exists("x"));
    CHECK_FALSE(table.exists("missing"));

    CHECK(table.symbol(x).name() == "x");
    CHECK(table.symbol<VariableSymbol>(x).kind() == Symbol::Kind::Variable);
    CHECK(table.symbol<VariableSymbol>(x).type() == *Types::real());
    CHECK(table.symbol<SubroutineSymbol>(foo).parameters() == std::vector<SymbolId>{x, p});
}

TEST_CASE("SymbolTable scoping", "[symbol]")
{
    SymbolTable table;

    table.declareVariable("x", Types::real());
    table.openScope();
    table.declareVariable("y", Types::text());
    table.declareVariable("x", Types::boolean());

    CHECK(table.lookup("y").has_value());
    CHECK(table.symbol<VariableSymbol>(*table.lookup("x")).type() == *Types::boolean());

    table.closeScope();

    CHECK(table.lookup("y") == std::nullopt);
    CHECK(table.symbol<VariableSymbol>(*table.lookup("x")).type() == *Types::real());
}
