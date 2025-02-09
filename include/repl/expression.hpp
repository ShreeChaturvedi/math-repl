#pragma once

#include <memory>
#include <ostream>
#include <variant>
#include <vector>

#include "repl/errors.hpp"
#include "repl/token.hpp"

namespace repl {

/** @brief Expression node categories used by the parser. */
enum class EType {
    Number,
    Variable,
    Unary,
    Binary,
    FnCall,
    Ternary,
};

std::ostream& operator<<(std::ostream& os, EType type);

struct Expression;
using ExpressionPtr = std::unique_ptr<Expression>;
using ExpressionList = std::vector<ExpressionPtr>;

/** @brief Unary operator node. */
struct UnaryNode {
    TType op;
    ExpressionPtr right;
};

/** @brief Binary operator node. */
struct BinaryNode {
    TType op;
    ExpressionPtr left;
    ExpressionPtr right;
};

/** @brief Function call node. */
struct FnNode {
    Identifier name;
    ExpressionList args;
};

/** @brief Ternary conditional node. */
struct TernaryNode {
    ExpressionPtr condition;
    ExpressionPtr then_branch;
    ExpressionPtr else_branch;
};

/** @brief Expression node container. */
struct Expression {
    EType type;
    std::variant<double, Identifier, UnaryNode, BinaryNode, FnNode, TernaryNode> data;

    template <typename T>
    const T& get() const {
        return std::get<T>(data);
    }

    template <typename T>
    T& get() {
        return std::get<T>(data);
    }
};

/** @brief Parse a token stream into an expression.
 *  @throws ParseError on invalid syntax.
 */
ExpressionPtr parse(TokenStream& stream);

/** @brief Parse tokens and ensure full consumption.
 *  @throws ParseError on invalid syntax or leftover tokens.
 */
ExpressionPtr parse(const Tokens& tokens);

/** @brief Create a numeric expression node. */
ExpressionPtr make_number(double value);
/** @brief Create a variable expression node. */
ExpressionPtr make_variable(Identifier name);
/** @brief Create a unary expression node. */
ExpressionPtr make_unary(TType op, ExpressionPtr right);
/** @brief Create a binary expression node. */
ExpressionPtr make_binary(TType op, ExpressionPtr left, ExpressionPtr right);
/** @brief Create a function call expression node. */
ExpressionPtr make_fn_call(Identifier name, ExpressionList args);
/** @brief Create a ternary expression node. */
ExpressionPtr make_ternary(ExpressionPtr condition, ExpressionPtr then_branch,
                           ExpressionPtr else_branch);

}  // namespace repl
