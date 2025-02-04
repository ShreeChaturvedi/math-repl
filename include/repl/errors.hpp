#pragma once

#include <stdexcept>
#include <string>

namespace repl {

/** @brief Error raised when parsing fails. */
class ParseError : public std::runtime_error {
public:
    explicit ParseError(const std::string& message) : std::runtime_error(message) {}
};

/** @brief Error raised when evaluation fails. */
class EvalError : public std::runtime_error {
public:
    explicit EvalError(const std::string& message) : std::runtime_error(message) {}
};

/** @brief Error raised for invalid REPL commands. */
class CommandError : public std::runtime_error {
public:
    explicit CommandError(const std::string& message) : std::runtime_error(message) {}
};

}  // namespace repl
