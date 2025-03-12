#include <catch2/catch_test_macros.hpp>

#include "repl/expression.hpp"
#include "repl/token.hpp"

using repl::BinaryNode;
using repl::EType;
using repl::TType;

TEST_CASE("Parser respects operator precedence") {
    auto expr = repl::parse(repl::tokenize("2 + 3 * 4"));
    REQUIRE(expr->type == EType::Binary);
    const auto& root = expr->get<BinaryNode>();
    REQUIRE(root.op == TType::Plus);
    REQUIRE(root.right->type == EType::Binary);
    REQUIRE(root.right->get<BinaryNode>().op == TType::Star);
}

TEST_CASE("Parser handles right-associative power") {
    auto expr = repl::parse(repl::tokenize("2 ^ 3 ^ 2"));
    REQUIRE(expr->type == EType::Binary);
    const auto& root = expr->get<BinaryNode>();
    REQUIRE(root.op == TType::Caret);
    REQUIRE(root.right->type == EType::Binary);
    REQUIRE(root.right->get<BinaryNode>().op == TType::Caret);
}

TEST_CASE("Parser handles ternary expressions") {
    auto expr = repl::parse(repl::tokenize("1 ? 2 : 3"));
    REQUIRE(expr->type == EType::Ternary);
}

TEST_CASE("Parser rejects incomplete ternary expressions") {
    REQUIRE_THROWS_AS(repl::parse(repl::tokenize("1 ? 2")), repl::ParseError);
}

TEST_CASE("Parser builds right-associative assignment") {
    auto expr = repl::parse(repl::tokenize("a = b = 3"));
    REQUIRE(expr->type == EType::Binary);
    const auto& root = expr->get<BinaryNode>();
    REQUIRE(root.op == TType::Equals);
    REQUIRE(root.right->type == EType::Binary);
    REQUIRE(root.right->get<BinaryNode>().op == TType::Equals);
}
