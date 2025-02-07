#include "repl/token.hpp"

#include "repl/errors.hpp"

#include <cctype>
#include <format>
#include <sstream>
#include <stdexcept>

namespace repl {

Token Token::number(double value) {
    return Token{TType::Number, value};
}

Token Token::identifier(Identifier name) {
    return Token{TType::Identifier, std::move(name)};
}

std::string_view to_string(TType type) {
    switch (type) {
        case TType::Number: return "Number";
        case TType::Identifier: return "Identifier";
        case TType::Plus: return "Plus";
        case TType::Minus: return "Minus";
        case TType::Star: return "Star";
        case TType::Slash: return "Slash";
        case TType::Percent: return "Percent";
        case TType::Caret: return "Caret";
        case TType::LParen: return "LParen";
        case TType::RParen: return "RParen";
        case TType::Equals: return "Equals";
        case TType::EqualEqual: return "EqualEqual";
        case TType::BangEqual: return "BangEqual";
        case TType::Less: return "Less";
        case TType::LessEqual: return "LessEqual";
        case TType::Greater: return "Greater";
        case TType::GreaterEqual: return "GreaterEqual";
        case TType::Question: return "Question";
        case TType::Colon: return "Colon";
        case TType::Comma: return "Comma";
    }
    return "Unknown";
}

std::ostream& operator<<(std::ostream& os, TType type) {
    return os << to_string(type);
}

std::ostream& operator<<(std::ostream& os, const Token& token) {
    os << token.type;
    if (token.type == TType::Number) {
        os << '[' << token.get<double>() << ']';
    }
    if (token.type == TType::Identifier) {
        os << '[' << token.get<Identifier>() << ']';
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Tokens& tokens) {
    if (tokens.empty()) {
        return os << "[]";
    }
    os << '[' << tokens.front();
    for (std::size_t index = 1; index < tokens.size(); ++index) {
        os << ", " << tokens[index];
    }
    return os << ']';
}

namespace {

bool is_digit(char ch) {
    return std::isdigit(static_cast<unsigned char>(ch)) != 0;
}

bool is_alpha(char ch) {
    return std::isalpha(static_cast<unsigned char>(ch)) != 0;
}

bool is_alnum(char ch) {
    return std::isalnum(static_cast<unsigned char>(ch)) != 0;
}

}  // namespace

Tokens tokenize(std::string_view input) {
    Tokens result;
    result.reserve(input.size());

    for (std::size_t pos = 0; pos < input.size(); ++pos) {
        char c = input[pos];
        if (std::isspace(static_cast<unsigned char>(c)) != 0) {
            continue;
        }

        if (c == '+') {
            result.push_back({TType::Plus, {}});
            continue;
        }
        if (c == '-') {
            result.push_back({TType::Minus, {}});
            continue;
        }
        if (c == '*') {
            result.push_back({TType::Star, {}});
            continue;
        }
        if (c == '/') {
            result.push_back({TType::Slash, {}});
            continue;
        }
        if (c == '%') {
            result.push_back({TType::Percent, {}});
            continue;
        }
        if (c == '^') {
            result.push_back({TType::Caret, {}});
            continue;
        }
        if (c == '(') {
            result.push_back({TType::LParen, {}});
            continue;
        }
        if (c == ')') {
            result.push_back({TType::RParen, {}});
            continue;
        }
        if (c == ',') {
            result.push_back({TType::Comma, {}});
            continue;
        }
        if (c == '?') {
            result.push_back({TType::Question, {}});
            continue;
        }
        if (c == ':') {
            result.push_back({TType::Colon, {}});
            continue;
        }

        if (c == '=') {
            if (pos + 1 < input.size() && input[pos + 1] == '=') {
                result.push_back({TType::EqualEqual, {}});
                ++pos;
            } else {
                result.push_back({TType::Equals, {}});
            }
            continue;
        }
        if (c == '!') {
            if (pos + 1 < input.size() && input[pos + 1] == '=') {
                result.push_back({TType::BangEqual, {}});
                ++pos;
            } else {
                throw ParseError(std::format(
                    "Unexpected '!' at position {}. Did you mean '!='?", pos));
            }
            continue;
        }
        if (c == '<') {
            if (pos + 1 < input.size() && input[pos + 1] == '=') {
                result.push_back({TType::LessEqual, {}});
                ++pos;
            } else {
                result.push_back({TType::Less, {}});
            }
            continue;
        }
        if (c == '>') {
            if (pos + 1 < input.size() && input[pos + 1] == '=') {
                result.push_back({TType::GreaterEqual, {}});
                ++pos;
            } else {
                result.push_back({TType::Greater, {}});
            }
            continue;
        }

        if (is_digit(c) || (c == '.' && pos + 1 < input.size() && is_digit(input[pos + 1]))) {
            std::size_t start = pos;
            bool seen_dot = false;

            while (pos < input.size()) {
                char current = input[pos];
                if (current == '.') {
                    if (seen_dot) {
                        break;
                    }
                    seen_dot = true;
                    ++pos;
                    continue;
                }
                if (!is_digit(current)) {
                    break;
                }
                ++pos;
            }

            std::string number_text{input.substr(start, pos - start)};
            result.push_back(Token::number(std::stod(number_text)));
            --pos;
            continue;
        }

        if (is_alpha(c) || c == '_') {
            std::size_t start = pos;
            while (pos < input.size() && (is_alnum(input[pos]) || input[pos] == '_')) {
                ++pos;
            }
            result.push_back(Token::identifier(std::string{input.substr(start, pos - start)}));
            --pos;
            continue;
        }

        throw ParseError(std::format(
            "Could not parse character '{}' at position {}", c, pos));
    }

    return result;
}

TokenStream::TokenStream(const Tokens& tokens) : tokens_(tokens), pos_(0) {}

const Token& TokenStream::peek() const {
    if (empty()) {
        throw std::underflow_error("Cannot peek empty token stream");
    }
    return tokens_[pos_];
}

const Token& TokenStream::get() {
    if (empty()) {
        throw std::underflow_error("Cannot get from empty token stream");
    }
    const auto& current = tokens_[pos_];
    ++pos_;
    return current;
}

bool TokenStream::match(TType type) {
    if (!empty() && peek().type == type) {
        get();
        return true;
    }
    return false;
}

const Token& TokenStream::expect(TType type) {
    if (empty()) {
        throw std::underflow_error("Cannot read from empty token stream");
    }
    if (peek().type != type) {
        throw ParseError(std::format(
            "Expected token {} but found {}", to_string(type), to_string(peek().type)));
    }
    return get();
}

std::size_t TokenStream::remaining() const {
    return tokens_.size() - pos_;
}

bool TokenStream::empty() const {
    return remaining() == 0;
}

}  // namespace repl
