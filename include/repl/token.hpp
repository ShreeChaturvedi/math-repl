#pragma once

#include <cstddef>
#include <ostream>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace repl {

/** @brief Token categories produced by the lexer. */
enum class TType {
    Number,
    Identifier,
    Plus,
    Minus,
    Star,
    Slash,
    Percent,
    Caret,
    LParen,
    RParen,
    Equals,
    EqualEqual,
    BangEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    Question,
    Colon,
    Comma,
};

/** @brief Name type for variables and functions. */
using Identifier = std::string;
/** @brief Parameter list type for user-defined functions. */
using Identifiers = std::vector<Identifier>;

/** @brief Represents a single token with optional payload. */
struct Token {
    TType type;
    std::variant<std::monostate, double, Identifier> data;

    /** @brief Create a numeric token. */
    static Token number(double value);
    /** @brief Create an identifier token. */
    static Token identifier(Identifier name);

    template <typename T>
    const T& get() const {
        return std::get<T>(data);
    }
};

/** @brief Token sequence produced by the lexer. */
using Tokens = std::vector<Token>;

/** @brief Convert a token type to a display name. */
std::string_view to_string(TType type);

std::ostream& operator<<(std::ostream& os, TType type);
std::ostream& operator<<(std::ostream& os, const Token& token);
std::ostream& operator<<(std::ostream& os, const Tokens& tokens);

/** @brief Tokenize a source string into tokens.
 *  @throws ParseError on unknown characters.
 */
Tokens tokenize(std::string_view input);

/** @brief Lightweight stream for parsing tokens. */
class TokenStream {
public:
    explicit TokenStream(const Tokens& tokens);

    /** @brief Peek at the current token without consuming it. */
    const Token& peek() const;
    /** @brief Consume and return the current token. */
    const Token& get();
    /** @brief Consume the token if it matches the expected type. */
    bool match(TType type);
    /** @brief Consume the token or throw if it does not match.
     *  @throws std::underflow_error if the stream is empty.
     *  @throws ParseError if the type does not match.
     */
    const Token& expect(TType type);

    /** @brief Remaining token count. */
    std::size_t remaining() const;
    /** @brief Whether the stream is exhausted. */
    bool empty() const;

private:
    const Tokens& tokens_;
    std::size_t pos_;
};

}  // namespace repl
