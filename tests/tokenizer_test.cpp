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

TEST_CASE("Tokenize scientific notation") {
    auto tokens = repl::tokenize("1e3 2.5e-2 1E+10 6.02E23 .5e2");
    REQUIRE(tokens.size() == 5);
    REQUIRE(tokens[0].type == TType::Number);
    REQUIRE(tokens[0].get<double>() == 1000.0);
    REQUIRE(tokens[1].type == TType::Number);
    REQUIRE(tokens[1].get<double>() == 0.025);
    REQUIRE(tokens[2].type == TType::Number);
    REQUIRE(tokens[2].get<double>() == 1e10);
    REQUIRE(tokens[3].type == TType::Number);
    REQUIRE(tokens[3].get<double>() == 6.02e23);
    REQUIRE(tokens[4].type == TType::Number);
    REQUIRE(tokens[4].get<double>() == 50.0);
}

TEST_CASE("Tokenize rejects incomplete scientific notation") {
    REQUIRE_THROWS_AS(repl::tokenize("1e"), repl::ParseError);
    REQUIRE_THROWS_AS(repl::tokenize("2.5E+"), repl::ParseError);
    REQUIRE_THROWS_AS(repl::tokenize("3e-"), repl::ParseError);
}

TEST_CASE("Tokenize rejects multi-dot numbers") {
    REQUIRE_THROWS_AS(repl::tokenize("1.2.3"), repl::ParseError);
    REQUIRE_THROWS_AS(repl::tokenize(".1.2"), repl::ParseError);
    REQUIRE_THROWS_AS(repl::tokenize("0.0.0"), repl::ParseError);
}
