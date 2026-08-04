#include "types.hxx"
#include "symbol.hxx"

#include <catch2/catch_test_macros.hpp>

using namespace basic;

TEST_CASE("Nothing type names and equality", "[types]")
{
    CHECK(Types::nothing()->name() == "NOTHING");
    CHECK(Types::nothing()->kind() == Type::Kind::Nothing);

    CHECK(Types::nothing() == Types::nothing()); // singleton check
    CHECK(*Types::nothing() == *Types::nothing());
    CHECK_FALSE(*Types::nothing() == *Types::real());
    CHECK_FALSE(*Types::nothing() == *Types::array(Types::real()));
}

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
    SubroutineSymbol subSym{2, "foo", {Types::real(), Types::text()}, Types::boolean()};
    CHECK(subSym.id() == 2);
    CHECK(subSym.name() == "foo");
    CHECK(subSym.kind() == Symbol::Kind::Subroutine);

    REQUIRE(subSym.parameterTypes().size() == 2);
    CHECK(*subSym.parameterTypes()[0] == *Types::real());
    CHECK(*subSym.parameterTypes()[1] == *Types::text());
    REQUIRE(subSym.returnType().has_value());
    CHECK(**subSym.returnType() == *Types::boolean());

    // պրոցեդուրան (առանց AS) ունի NOTHING վերադարձի տիպ
    SubroutineSymbol procSym{3, "g", {}, Types::nothing()};
    REQUIRE(procSym.returnType().has_value());
    CHECK(**procSym.returnType() == *Types::nothing());
}

TEST_CASE("Types::fromKeyword maps type keywords", "[types]")
{
    CHECK(*Types::fromKeyword("REAL") == *Types::real());
    CHECK(*Types::fromKeyword("TEXT") == *Types::text());
    CHECK(*Types::fromKeyword("BOOL") == *Types::boolean());
    // անհայտ ծառայողական բառը տալիս է Nothing
    CHECK(*Types::fromKeyword("FOO") == *Types::nothing());
}

TEST_CASE("SymbolTable declarations and lookup", "[symbol]")
{
    SymbolTable table;

    SymbolId x = table.declareVariable("x", Types::real());
    SymbolId p = table.declareParameter("p", Types::text());
    SymbolId foo = table.declareSubroutine("foo", {}, Types::nothing());

    CHECK(table.lookup("x") == x);
    CHECK(table.lookup("p") == p);
    CHECK(table.lookup("foo") == foo);
    CHECK(table.lookup("missing") == std::nullopt);
    CHECK(table.exists("x"));
    CHECK_FALSE(table.exists("missing"));

    CHECK(table.symbol(x).name() == "x");
    CHECK(table.symbol<VariableSymbol>(x).kind() == Symbol::Kind::Variable);
    CHECK(table.symbol<VariableSymbol>(x).type() == *Types::real());
    CHECK(table.symbol<SubroutineSymbol>(foo).parameterTypes().empty());
}

TEST_CASE("lookupSubroutine ignores shadowing variables", "[symbol]")
{
    SymbolTable table;

    table.declareSubroutine("f", {}, Types::nothing());
    CHECK(table.lookupSubroutine("f").has_value());

    // նույն անունով փոփոխականը չի խանգարում ենթածրագիր գտնելուն
    table.openScope();
    table.declareVariable("f", Types::real());
    CHECK(table.lookupSubroutine("f").has_value());
    CHECK(table.symbol<SubroutineSymbol>(*table.lookupSubroutine("f")).name() == "f");

    // իսկ սովորական lookup-ը գտնում է մոտակա (փոփոխականի) սիմվոլը
    CHECK(table.symbol<VariableSymbol>(*table.lookup("f")).type() == *Types::real());

    table.closeScope();
    CHECK(table.lookupSubroutine("f").has_value());
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
