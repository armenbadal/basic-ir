#include "aslisp.hxx"
#include "ast.hxx"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <sstream>
#include <memory>
#include <string>

using namespace basic;
using Catch::Matchers::StartsWith;
using Catch::Matchers::EndsWith;
using Catch::Matchers::ContainsSubstring;

static Program::Ptr makeProg(std::vector<Subroutine::Ptr> subs)
{
    return std::make_shared<Program>(std::move(subs), 0);
}

static Subroutine::Ptr makeSub(std::string_view name, std::vector<std::string> params, Statement::Ptr body)
{
    std::vector<Dim::Ptr> dims;
    for (auto& p : params)
        dims.push_back(std::make_shared<Dim>(p, nullptr, "REAL", 1));
    return std::make_shared<Subroutine>(name, std::move(dims), "empty", std::move(body), 1);
}

static Sequence::Ptr makeSeq(std::vector<Statement::Ptr> items)
{
    return std::make_shared<Sequence>(std::move(items), 2);
}

static Let::Ptr makeLet(std::string_view var, Expression::Ptr expr)
{
    return std::make_shared<Let>(std::make_shared<Variable>(var, 4), std::move(expr), 5);
}

static If::Ptr makeIf(Expression::Ptr cond, Statement::Ptr thenSt, Statement::Ptr elseSt)
{
    auto branch = std::make_shared<If::IfThen>(std::move(cond), std::move(thenSt), 6);
    return std::make_shared<If>(std::vector<If::IfThen::Ptr>{branch}, std::move(elseSt), 6);
}

static For::Ptr makeFor(std::string_view var, Expression::Ptr begin, Expression::Ptr end, Number::Ptr step, Statement::Ptr body)
{
    return std::make_shared<For>(std::make_shared<Variable>(var, 7), std::move(begin), std::move(end), std::move(step), std::move(body), 8);
}

static While::Ptr makeWhile(Expression::Ptr cond, Statement::Ptr body)
{
    return std::make_shared<While>(std::move(cond), std::move(body), 9);
}

static Call::Ptr makeCall(std::string_view callee, std::vector<Expression::Ptr> args)
{
    return std::make_shared<Call>(callee, std::move(args), 10);
}

static Dim::Ptr makeDim(std::string_view name, Expression::Ptr size, std::string_view type)
{
    return std::make_shared<Dim>(name, std::move(size), type, 13);
}

static std::string tos(Program::Ptr prog)
{
    std::ostringstream ss;
    Lisper{}.emit(prog, ss);
    return ss.str();
}

TEST_CASE("Empty program", "[lisp]")
{
    CHECK(tos(makeProg({})) == "(basic-program :subroutines)\n");
}

TEST_CASE("Program with one empty subroutine", "[lisp]")
{
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({}))}));
    CHECK_THAT(result, StartsWith("(basic-program :subroutines (basic-subroutine :name \"Main\""));
    CHECK_THAT(result, ContainsSubstring("'()"));
    CHECK_THAT(result, ContainsSubstring("(basic-sequence :items)"));
    CHECK_THAT(result, EndsWith(")\n"));
}

TEST_CASE("Program with multiple subroutines", "[lisp]")
{
    auto empty = makeSeq({});
    auto result = tos(makeProg({makeSub("A", {}, empty), makeSub("B", {}, empty)}));
    CHECK_THAT(result, ContainsSubstring("(basic-subroutine :name \"A\""));
    CHECK_THAT(result, ContainsSubstring("(basic-subroutine :name \"B\""));
}

TEST_CASE("Subroutine with parameters", "[lisp]")
{
    auto result = tos(makeProg({makeSub("max", {"x", "y"}, makeSeq({}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-subroutine :name \"max\""));
    CHECK_THAT(result, ContainsSubstring("'((basic-dim :name \"x\" :size NIL :type \"REAL\") (basic-dim :name \"y\" :size NIL :type \"REAL\"))"));
}

TEST_CASE("Subroutine with sized parameters", "[lisp]")
{
    std::vector<Dim::Ptr> params;
    params.push_back(std::make_shared<Dim>("a", node<Number>(10.0, 1), "REAL", 1));
    params.push_back(std::make_shared<Dim>("b", nullptr, "TEXT", 1));
    auto sub = std::make_shared<Subroutine>("f", std::move(params), "empty", makeSeq({}), 1);
    auto result = tos(makeProg({sub}));
    CHECK_THAT(result, ContainsSubstring("'((basic-dim :name \"a\" :size (basic-number :value 10) :type \"REAL\") (basic-dim :name \"b\" :size NIL :type \"TEXT\"))"));
}

TEST_CASE("Let assign number", "[lisp]")
{
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makeLet("x", node<Number>(42.0, 1))}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-let (basic-variable :name \"x\") (basic-number :value 42))"));
}

TEST_CASE("Let assign text", "[lisp]")
{
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makeLet("x", node<Text>("hello", 1))}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-let (basic-variable :name \"x\") (basic-text :value \"hello\"))"));
}

TEST_CASE("Let assign boolean", "[lisp]")
{
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makeLet("flag", node<Boolean>(true, 1))}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-let (basic-variable :name \"flag\") (basic-boolean :value T))"));
}

TEST_CASE("Binary addition", "[lisp]")
{
    auto expr = node<Binary>(Operation::Add, node<Number>(1.0, 1), node<Number>(2.0, 2), 3);
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makeLet("x", expr)}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-binary :operation \"ADD\" :left (basic-number :value 1) :right (basic-number :value 2))"));
}

TEST_CASE("Binary comparison", "[lisp]")
{
    auto expr = node<Binary>(Operation::Gt, node<Variable>("x", 1), node<Number>(0.0, 2), 3);
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makeLet("r", expr)}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-binary :operation \"GT\" :left (basic-variable :name \"x\") :right (basic-number :value 0))"));
}

TEST_CASE("Unary NOT", "[lisp]")
{
    auto expr = node<Unary>(Operation::Not, node<Boolean>(false, 1), 2);
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makeLet("x", expr)}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-unary :operation \"NOT\" :operand (basic-boolean :value NIL))"));
}

TEST_CASE("Unary negation", "[lisp]")
{
    auto expr = node<Unary>(Operation::Sub, node<Number>(5.0, 1), 2);
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makeLet("x", expr)}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-unary :operation \"SUB\" :operand (basic-number :value 5))"));
}

TEST_CASE("If without else", "[lisp]")
{
    auto cond = node<Boolean>(true, 1);
    auto thenS = makeLet("x", node<Number>(1.0, 2));
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makeIf(cond, thenS, nullptr)}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-if :branches (basic-if-then :condition (basic-boolean :value T) :decision (basic-let (basic-variable :name \"x\") (basic-number :value 1))) :alternative)"));
}

TEST_CASE("If with else", "[lisp]")
{
    auto cond = node<Boolean>(false, 1);
    auto thenS = makeLet("x", node<Number>(1.0, 2));
    auto elseS = makeLet("y", node<Number>(2.0, 3));
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makeIf(cond, thenS, elseS)}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-if :branches (basic-if-then :condition (basic-boolean :value NIL) :decision (basic-let (basic-variable :name \"x\") (basic-number :value 1))) :alternative (basic-let (basic-variable :name \"y\") (basic-number :value 2)))"));
}

TEST_CASE("While loop", "[lisp]")
{
    auto cond = node<Binary>(Operation::Lt, node<Variable>("i", 1), node<Number>(10.0, 2), 3);
    auto body = makeLet("x", node<Variable>("i", 4));
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makeWhile(cond, body)}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-while :condition (basic-binary :operation \"LT\" :left (basic-variable :name \"i\") :right (basic-number :value 10)) :body (basic-let (basic-variable :name \"x\") (basic-variable :name \"i\")))"));
}

TEST_CASE("For loop", "[lisp]")
{
    auto body = makeLet("x", node<Number>(0.0, 1));
    auto f = makeFor("i", node<Number>(1.0, 2), node<Number>(10.0, 3), node<Number>(1.0, 4), body);
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({f}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-for :parameter (basic-variable :name \"i\") :begin (basic-number :value 1) :end (basic-number :value 10) :step (basic-number :value 1) :body (basic-let (basic-variable :name \"x\") (basic-number :value 0)))"));
}

TEST_CASE("Call subroutine", "[lisp]")
{
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makeCall("foo", {node<Number>(1.0, 1), node<Text>("bar", 2)})}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-call :callee \"foo\" :arguments (basic-number :value 1) (basic-text :value \"bar\"))"));
}

TEST_CASE("Dim statement", "[lisp]")
{
    auto d = makeDim("arr", node<Number>(100.0, 1), "REAL");
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({d}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-dim :name \"arr\" :size (basic-number :value 100) :type \"REAL\")"));
}

TEST_CASE("Dim statement without size", "[lisp]")
{
    auto d = makeDim("arr", nullptr, "REAL");
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({d}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-dim :name \"arr\" :size NIL :type \"REAL\")"));
}

TEST_CASE("Array expression", "[lisp]")
{
    auto arr = node<Array>(std::vector<Expression::Ptr>{node<Number>(1.0, 1), node<Number>(2.0, 2)}, 3);
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makeLet("x", arr)}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-array :elements (basic-number :value 1) (basic-number :value 2))"));
}

TEST_CASE("Apply function", "[lisp]")
{
    auto ap = node<Apply>("ABS", std::vector<Expression::Ptr>{node<Number>(-5.0, 1)}, 2);
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makeLet("x", ap)}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-apply :callee \"ABS\" :arguments (basic-number :value -5))"));
}

TEST_CASE("Multiple statements", "[lisp]")
{
    auto s1 = makeLet("x", node<Number>(1.0, 1));
    auto s2 = makeLet("y", node<Number>(2.0, 2));
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({s1, s2}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-let (basic-variable :name \"x\") (basic-number :value 1))"));
    CHECK_THAT(result, ContainsSubstring("(basic-let (basic-variable :name \"y\") (basic-number :value 2))"));
}

TEST_CASE("Power operator", "[lisp]")
{
    auto expr = node<Binary>(Operation::Pow, node<Number>(2.0, 1), node<Number>(3.0, 2), 3);
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makeLet("x", expr)}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-binary :operation \"POW\" :left (basic-number :value 2) :right (basic-number :value 3))"));
}

TEST_CASE("Mod operator", "[lisp]")
{
    auto expr = node<Binary>(Operation::Mod, node<Number>(7.0, 1), node<Number>(3.0, 2), 3);
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makeLet("x", expr)}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-binary :operation \"MOD\" :left (basic-number :value 7) :right (basic-number :value 3))"));
}

TEST_CASE("Nested binary expressions", "[lisp]")
{
    auto ab = node<Binary>(Operation::Add, node<Number>(1.0, 1), node<Number>(2.0, 2), 4);
    auto expr = node<Binary>(Operation::Mul, ab, node<Number>(3.0, 3), 5);
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makeLet("x", expr)}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-binary :operation \"MUL\" :left (basic-binary :operation \"ADD\" :left (basic-number :value 1) :right (basic-number :value 2)) :right (basic-number :value 3))"));
}

TEST_CASE("Variable reference", "[lisp]")
{
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makeLet("x", node<Variable>("result", 1))}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-let (basic-variable :name \"x\") (basic-variable :name \"result\"))"));
}

TEST_CASE("Boolean true", "[lisp]")
{
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makeLet("x", node<Boolean>(true, 1))}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-let (basic-variable :name \"x\") (basic-boolean :value T))"));
}

TEST_CASE("Boolean false", "[lisp]")
{
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makeLet("x", node<Boolean>(false, 1))}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-let (basic-variable :name \"x\") (basic-boolean :value NIL))"));
}

TEST_CASE("Concatenation operator", "[lisp]")
{
    auto expr = node<Binary>(Operation::Conc, node<Text>("a", 1), node<Text>("b", 2), 3);
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makeLet("x", expr)}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-binary :operation \"CONC\" :left (basic-text :value \"a\") :right (basic-text :value \"b\"))"));
}

TEST_CASE("AND operator", "[lisp]")
{
    auto expr = node<Binary>(Operation::And, node<Boolean>(true, 1), node<Boolean>(false, 2), 3);
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makeLet("x", expr)}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-binary :operation \"AND\" :left (basic-boolean :value T) :right (basic-boolean :value NIL))"));
}

TEST_CASE("OR operator", "[lisp]")
{
    auto expr = node<Binary>(Operation::Or, node<Boolean>(true, 1), node<Boolean>(false, 2), 3);
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makeLet("x", expr)}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-binary :operation \"OR\" :left (basic-boolean :value T) :right (basic-boolean :value NIL))"));
}

TEST_CASE("Empty call", "[lisp]")
{
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makeCall("foo", {})}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-call :callee \"foo\" :arguments)"));
}

TEST_CASE("Number with decimal", "[lisp]")
{
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makeLet("x", node<Number>(3.14159, 1))}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-number :value 3.14159)"));
}

TEST_CASE("Text with special characters", "[lisp]")
{
    auto t = node<Text>("line1\nline2", 1);
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makeLet("x", t)}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-text :value \"line1\nline2\")"));
}

TEST_CASE("emit writes to stream", "[lisp]")
{
    auto prog = makeProg({});
    std::ostringstream ss;
    Lisper{}.emit(prog, ss);
    CHECK(ss.str() == "(basic-program :subroutines)\n");
}
