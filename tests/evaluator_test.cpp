#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "repl/evaluator.hpp"
#include "repl/state.hpp"

using Catch::Approx;

TEST_CASE("Evaluator handles arithmetic and precedence") {
    repl::State state;
    auto result = repl::process_query("2 + 3 * 4", state);
    REQUIRE(result.value);
    REQUIRE(*result.value == Approx(14.0));
}

TEST_CASE("Evaluator handles power right-associativity") {
    repl::State state;
    auto result = repl::process_query("2 ^ 3 ^ 2", state);
    REQUIRE(result.value);
    REQUIRE(*result.value == Approx(512.0));
}

TEST_CASE("Evaluator supports built-in functions") {
    repl::State state;
    auto result = repl::process_query("cos(0)", state);
    REQUIRE(result.value);
    REQUIRE(*result.value == Approx(1.0));
}

TEST_CASE("Evaluator validates built-in arity") {
    repl::State state;
    REQUIRE_THROWS_AS(repl::process_query("sin(1, 2)", state), repl::EvalError);
}

TEST_CASE("Evaluator handles constants and last result") {
    repl::State state;
    auto result = repl::process_query("pi", state);
    REQUIRE(result.value);
    REQUIRE(*result.value == Approx(3.141592653589793));

    auto last = repl::process_query("_ * 2", state);
    REQUIRE(last.value);
    REQUIRE(*last.value == Approx(2 * 3.141592653589793));
}

TEST_CASE("Evaluator rejects assignment to read-only names") {
    repl::State state;
    REQUIRE_THROWS_AS(repl::process_query("pi = 3", state), repl::EvalError);
    REQUIRE_THROWS_AS(repl::process_query("_ = 2", state), repl::EvalError);
}

TEST_CASE("Evaluator handles relational and ternary operators") {
    repl::State state;
    auto result = repl::process_query("3 > 2 ? 10 : 20", state);
    REQUIRE(result.value);
    REQUIRE(*result.value == Approx(10.0));
}

TEST_CASE("Evaluator defines and calls user functions") {
    repl::State state;
    auto def = repl::process_query("square(x) = x * x", state);
    REQUIRE_FALSE(def.value.has_value());
    REQUIRE(def.info);

    auto call = repl::process_query("square(5)", state);
    REQUIRE(call.value);
    REQUIRE(*call.value == Approx(25.0));
}

TEST_CASE("Evaluator rejects duplicate parameters") {
    repl::State state;
    REQUIRE_THROWS_AS(repl::process_query("f(x, x) = x + x", state), repl::EvalError);
}

TEST_CASE("Evaluator supports recursion") {
    repl::State state;
    repl::process_query("fact(n) = n <= 1 ? 1 : n * fact(n - 1)", state);
    auto result = repl::process_query("fact(5)", state);
    REQUIRE(result.value);
    REQUIRE(*result.value == Approx(120.0));
}

TEST_CASE("Evaluator handles division and modulo by zero") {
    repl::State state;
    REQUIRE_THROWS_AS(repl::process_query("1 / 0", state), repl::EvalError);
    REQUIRE_THROWS_AS(repl::process_query("5 % 0", state), repl::EvalError);
}
