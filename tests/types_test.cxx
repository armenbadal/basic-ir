#include "types.hxx"
#include "symbol.hxx"

#include <catch2/catch_test_macros.hpp>

using namespace basic;

TEST_CASE("ScalarType names and equality", "[types]")
{
    auto intType1 = Types::integer();
    auto intType2 = Types::integer();
    auto realType = Types::real();
    auto boolType = Types::boolean();

    CHECK(intType1->name() == "INTEGER");
    CHECK(realType->name() == "REAL");
    CHECK(boolType->name() == "BOOLEAN");

    CHECK(intType1->kind() == Type::Kind::Scalar);
    CHECK(realType->kind() == Type::Kind::Scalar);
    CHECK(boolType->kind() == Type::Kind::Scalar);

    CHECK(*intType1 == *intType2);
    CHECK(intType1 == intType2); // singleton check
    CHECK_FALSE(*intType1 == *realType);
    CHECK_FALSE(*intType1 == *boolType);
}

TEST_CASE("ArrayType names, equality, and elementType", "[types]")
{
    auto intArray1 = Types::array(Types::integer());
    auto intArray2 = Types::array(Types::integer());
    auto realArray = Types::array(Types::real());

    CHECK(intArray1->name() == "ARRAY");
    CHECK(intArray1->kind() == Type::Kind::Array);

    CHECK(*intArray1 == *intArray2);
    CHECK_FALSE(*intArray1 == *realArray);

    auto arrayTypePtr = std::dynamic_pointer_cast<const ArrayType>(intArray1);
    REQUIRE(arrayTypePtr != nullptr);
    CHECK(arrayTypePtr->elementType() == *Types::integer());
}

TEST_CASE("VariableSymbol functionality", "[symbol]")
{
    VariableSymbol varSym{1, "x", Types::integer()};
    CHECK(varSym.id() == 1);
    CHECK(varSym.name() == "x");
    CHECK(varSym.kind() == Symbol::Kind::Variable);
    CHECK(varSym.type() == *Types::integer());
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

    SymbolId x = table.declareVariable("x", Types::integer());
    SymbolId p = table.declareParameter("p", Types::real());
    SymbolId foo = table.declareSubroutine("foo", {x, p});

    CHECK(table.lookup("x") == x);
    CHECK(table.lookup("p") == p);
    CHECK(table.lookup("foo") == foo);
    CHECK(table.lookup("missing") == std::nullopt);
    CHECK(table.exists("x"));
    CHECK_FALSE(table.exists("missing"));

    CHECK(table.symbol(x).name() == "x");
    CHECK(table.symbol<VariableSymbol>(x).kind() == Symbol::Kind::Variable);
    CHECK(table.symbol<VariableSymbol>(x).type() == *Types::integer());
    CHECK(table.symbol<SubroutineSymbol>(foo).parameters() == std::vector<SymbolId>{x, p});
}

TEST_CASE("SymbolTable scoping", "[symbol]")
{
    SymbolTable table;

    table.declareVariable("x", Types::integer());
    table.openScope();
    table.declareVariable("y", Types::real());
    table.declareVariable("x", Types::boolean());

    CHECK(table.lookup("y").has_value());
    CHECK(table.symbol<VariableSymbol>(*table.lookup("x")).type() == *Types::boolean());

    table.closeScope();

    CHECK(table.lookup("y") == std::nullopt);
    CHECK(table.symbol<VariableSymbol>(*table.lookup("x")).type() == *Types::integer());
}
