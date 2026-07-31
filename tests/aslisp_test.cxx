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
    std::vector<Variable::Ptr> vars;
    for (auto& p : params)
        vars.push_back(std::make_shared<Variable>(p, 1));
    return std::make_shared<Subroutine>(name, std::move(vars), std::move(body), 1);
}

static Sequence::Ptr makeSeq(std::vector<Statement::Ptr> items)
{
    return std::make_shared<Sequence>(std::move(items), 2);
}

static Print::Ptr makePrint(Expression::Ptr expr)
{
    return std::make_shared<Print>(std::move(expr), 3);
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

static Input::Ptr makeInput(std::string_view var)
{
    return std::make_shared<Input>(std::make_shared<Variable>(var, 11), 12);
}

static Dim::Ptr makeDim(std::string_view name, Expression::Ptr size)
{
    return std::make_shared<Dim>(name, std::move(size), 13);
}

static std::string tos(Program::Ptr prog)
{
    std::ostringstream ss;
    Lisper{}.emit(prog, ss);
    return ss.str();
}

TEST_CASE("Empty program", "[lisp]")
{
    CHECK(tos(makeProg({})) == "(basic-program)\n");
}

TEST_CASE("Program with one empty subroutine", "[lisp]")
{
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({}))}));
    CHECK_THAT(result, StartsWith("(basic-program (basic-subroutine \"Main\""));
    CHECK_THAT(result, ContainsSubstring("'()"));
    CHECK_THAT(result, ContainsSubstring("(basic-sequence)"));
    CHECK_THAT(result, EndsWith(")\n"));
}

TEST_CASE("Program with multiple subroutines", "[lisp]")
{
    auto empty = makeSeq({});
    auto result = tos(makeProg({makeSub("A", {}, empty), makeSub("B", {}, empty)}));
    CHECK_THAT(result, ContainsSubstring("(basic-subroutine \"A\""));
    CHECK_THAT(result, ContainsSubstring("(basic-subroutine \"B\""));
}

TEST_CASE("Subroutine with parameters", "[lisp]")
{
    auto result = tos(makeProg({makeSub("max", {"x", "y"}, makeSeq({}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-subroutine \"max\""));
    CHECK_THAT(result, ContainsSubstring("'((basic-variable \"x\") (basic-variable \"y\"))"));
}

TEST_CASE("Print number", "[lisp]")
{
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makePrint(node<Number>(42.0, 1))}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-print (basic-number 42))"));
}

TEST_CASE("Print text", "[lisp]")
{
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makePrint(node<Text>("hello", 1))}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-print (basic-text \"hello\"))"));
}

TEST_CASE("Let assign number", "[lisp]")
{
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makeLet("x", node<Number>(10.0, 1))}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-let (basic-variable \"x\") (basic-number 10))"));
}

TEST_CASE("Let assign boolean", "[lisp]")
{
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makeLet("flag", node<Boolean>(true, 1))}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-let (basic-variable \"flag\") (basic-boolean T))"));
}

TEST_CASE("Binary addition", "[lisp]")
{
    auto expr = node<Binary>(Operation::Add, node<Number>(1.0, 1), node<Number>(2.0, 2), 3);
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makeLet("x", expr)}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-binary \"ADD\" (basic-number 1) (basic-number 2))"));
}

TEST_CASE("Binary comparison", "[lisp]")
{
    auto expr = node<Binary>(Operation::Gt, node<Variable>("x", 1), node<Number>(0.0, 2), 3);
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makeLet("r", expr)}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-binary \"GT\" (basic-variable \"x\") (basic-number 0))"));
}

TEST_CASE("Unary NOT", "[lisp]")
{
    auto expr = node<Unary>(Operation::Not, node<Boolean>(false, 1), 2);
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makePrint(expr)}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-unary \"NOT\" (basic-boolean NIL))"));
}

TEST_CASE("Unary negation", "[lisp]")
{
    auto expr = node<Unary>(Operation::Sub, node<Number>(5.0, 1), 2);
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makePrint(expr)}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-unary \"SUB\" (basic-number 5))"));
}

TEST_CASE("If without else", "[lisp]")
{
    auto cond = node<Boolean>(true, 1);
    auto thenP = makePrint(node<Text>("yes", 2));
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makeIf(cond, thenP, nullptr)}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-if (basic-if-then (basic-boolean T) (basic-print (basic-text \"yes\"))))"));
}

TEST_CASE("If with else", "[lisp]")
{
    auto cond = node<Boolean>(false, 1);
    auto thenP = makePrint(node<Text>("t", 2));
    auto elseP = makePrint(node<Text>("f", 3));
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makeIf(cond, thenP, elseP)}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-if (basic-if-then (basic-boolean NIL) (basic-print (basic-text \"t\"))) (basic-print (basic-text \"f\")))"));
}

TEST_CASE("While loop", "[lisp]")
{
    auto cond = node<Binary>(Operation::Lt, node<Variable>("i", 1), node<Number>(10.0, 2), 3);
    auto body = makePrint(node<Variable>("i", 4));
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makeWhile(cond, body)}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-while (basic-binary \"LT\" (basic-variable \"i\") (basic-number 10)) (basic-print (basic-variable \"i\")))"));
}

TEST_CASE("For loop", "[lisp]")
{
    auto body = makePrint(node<Number>(0.0, 1));
    auto f = makeFor("i", node<Number>(1.0, 2), node<Number>(10.0, 3), node<Number>(1.0, 4), body);
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({f}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-for (basic-variable \"i\") (basic-number 1) (basic-number 10) (basic-number 1) (basic-print (basic-number 0)))"));
}

TEST_CASE("Call subroutine", "[lisp]")
{
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makeCall("foo", {node<Number>(1.0, 1), node<Text>("bar", 2)})}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-call \"foo\" (basic-number 1) (basic-text \"bar\"))"));
}

TEST_CASE("Input statement", "[lisp]")
{
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makeInput("x")}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-input (basic-variable \"x\"))"));
}

TEST_CASE("Dim statement", "[lisp]")
{
    auto d = makeDim("arr", node<Number>(100.0, 1));
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({d}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-dim \"arr\" (basic-number 100))"));
}

TEST_CASE("Array expression", "[lisp]")
{
    auto arr = node<Array>(std::vector<Expression::Ptr>{node<Number>(1.0, 1), node<Number>(2.0, 2)}, 3);
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makePrint(arr)}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-array (basic-number 1) (basic-number 2))"));
}

TEST_CASE("Apply function", "[lisp]")
{
    auto ap = node<Apply>("ABS", std::vector<Expression::Ptr>{node<Number>(-5.0, 1)}, 2);
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makePrint(ap)}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-apply \"ABS\" (basic-number -5))"));
}

TEST_CASE("Multiple statements", "[lisp]")
{
    auto s1 = makeLet("x", node<Number>(1.0, 1));
    auto s2 = makeLet("y", node<Number>(2.0, 2));
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({s1, s2}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-let (basic-variable \"x\") (basic-number 1))"));
    CHECK_THAT(result, ContainsSubstring("(basic-let (basic-variable \"y\") (basic-number 2))"));
}

TEST_CASE("Power operator", "[lisp]")
{
    auto expr = node<Binary>(Operation::Pow, node<Number>(2.0, 1), node<Number>(3.0, 2), 3);
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makePrint(expr)}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-binary \"POW\" (basic-number 2) (basic-number 3))"));
}

TEST_CASE("Mod operator", "[lisp]")
{
    auto expr = node<Binary>(Operation::Mod, node<Number>(7.0, 1), node<Number>(3.0, 2), 3);
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makePrint(expr)}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-binary \"MOD\" (basic-number 7) (basic-number 3))"));
}

TEST_CASE("Nested binary expressions", "[lisp]")
{
    auto ab = node<Binary>(Operation::Add, node<Number>(1.0, 1), node<Number>(2.0, 2), 4);
    auto expr = node<Binary>(Operation::Mul, ab, node<Number>(3.0, 3), 5);
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makePrint(expr)}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-binary \"MUL\" (basic-binary \"ADD\" (basic-number 1) (basic-number 2)) (basic-number 3))"));
}

TEST_CASE("Variable reference", "[lisp]")
{
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makePrint(node<Variable>("result", 1))}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-print (basic-variable \"result\"))"));
}

TEST_CASE("Boolean true", "[lisp]")
{
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makePrint(node<Boolean>(true, 1))}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-print (basic-boolean T))"));
}

TEST_CASE("Boolean false", "[lisp]")
{
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makePrint(node<Boolean>(false, 1))}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-print (basic-boolean NIL))"));
}

TEST_CASE("Concatenation operator", "[lisp]")
{
    auto expr = node<Binary>(Operation::Conc, node<Text>("a", 1), node<Text>("b", 2), 3);
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makePrint(expr)}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-binary \"CONC\" (basic-text \"a\") (basic-text \"b\"))"));
}

TEST_CASE("AND operator", "[lisp]")
{
    auto expr = node<Binary>(Operation::And, node<Boolean>(true, 1), node<Boolean>(false, 2), 3);
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makePrint(expr)}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-binary \"AND\" (basic-boolean T) (basic-boolean NIL))"));
}

TEST_CASE("OR operator", "[lisp]")
{
    auto expr = node<Binary>(Operation::Or, node<Boolean>(true, 1), node<Boolean>(false, 2), 3);
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makePrint(expr)}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-binary \"OR\" (basic-boolean T) (basic-boolean NIL))"));
}

TEST_CASE("Empty call", "[lisp]")
{
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makeCall("foo", {})}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-call \"foo\")"));
}

TEST_CASE("Number with decimal", "[lisp]")
{
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makePrint(node<Number>(3.14159, 1))}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-number 3.14159)"));
}

TEST_CASE("Text with special characters", "[lisp]")
{
    auto t = node<Text>("line1\nline2", 1);
    auto result = tos(makeProg({makeSub("Main", {}, makeSeq({makePrint(t)}))}));
    CHECK_THAT(result, ContainsSubstring("(basic-text \"line1\nline2\")"));
}

TEST_CASE("emit writes to stream", "[lisp]")
{
    auto prog = makeProg({});
    std::ostringstream ss;
    Lisper{}.emit(prog, ss);
    CHECK(ss.str() == "(basic-program)\n");
}
