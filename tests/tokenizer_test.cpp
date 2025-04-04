#include <catch2/catch_test_macros.hpp>

#include "repl/errors.hpp"
#include "repl/token.hpp"

using repl::TType;

TEST_CASE("Tokenize numbers, identifiers, and operators") {
    auto tokens = repl::tokenize("sin(x) + 1.5<=2 ? y_1 : .25");

    REQUIRE(tokens.size() == 12);
    REQUIRE(tokens[0].type == TType::Identifier);
    REQUIRE(tokens[1].type == TType::LParen);
    REQUIRE(tokens[2].type == TType::Identifier);
    REQUIRE(tokens[3].type == TType::RParen);
    REQUIRE(tokens[4].type == TType::Plus);
    REQUIRE(tokens[5].type == TType::Number);
    REQUIRE(tokens[6].type == TType::LessEqual);
    REQUIRE(tokens[7].type == TType::Number);
    REQUIRE(tokens[8].type == TType::Question);
    REQUIRE(tokens[9].type == TType::Identifier);
    REQUIRE(tokens[10].type == TType::Colon);
    REQUIRE(tokens[11].type == TType::Number);
}

TEST_CASE("Tokenize multi-character comparisons") {
    auto tokens = repl::tokenize("a==b!=c<=d>=e<f>g");
    REQUIRE(tokens.size() == 13);
    REQUIRE(tokens[1].type == TType::EqualEqual);
    REQUIRE(tokens[3].type == TType::BangEqual);
    REQUIRE(tokens[5].type == TType::LessEqual);
    REQUIRE(tokens[7].type == TType::GreaterEqual);
    REQUIRE(tokens[9].type == TType::Less);
    REQUIRE(tokens[11].type == TType::Greater);
}

TEST_CASE("Tokenize rejects unexpected characters") {
    REQUIRE_THROWS_AS(repl::tokenize("2 @ 3"), repl::ParseError);
}
