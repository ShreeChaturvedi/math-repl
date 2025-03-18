#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "repl/evaluator.hpp"
#include "repl/state.hpp"

using Catch::Approx;

TEST_CASE("Integration: state evolves across queries") {
    repl::State state;

    repl::process_query("x = 2", state);
    repl::process_query("f(y) = y + x", state);

    auto first = repl::process_query("f(3)", state);
    REQUIRE(first.value);
    REQUIRE(*first.value == Approx(5.0));

    repl::process_query("x = 10", state);
    auto second = repl::process_query("f(3)", state);
    REQUIRE(second.value);
    REQUIRE(*second.value == Approx(13.0));
}

TEST_CASE("Integration: assignments inside functions are local") {
    repl::State state;

    repl::process_query("y = 1", state);
    repl::process_query("set_local() = y = 5", state);

    auto result = repl::process_query("set_local()", state);
    REQUIRE(result.value);
    REQUIRE(*result.value == Approx(5.0));

    auto global = repl::process_query("y", state);
    REQUIRE(global.value);
    REQUIRE(*global.value == Approx(1.0));
}
