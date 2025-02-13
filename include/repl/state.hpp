#pragma once

#include <functional>
#include <ostream>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "repl/expression.hpp"

namespace repl {

/** @brief Map of variable values. */
using VariableMap = std::unordered_map<Identifier, double>;

/** @brief User-defined function data. */
struct FnObj {
    Identifiers params;
    ExpressionPtr expr;
};

/** @brief User-defined function table. */
using UserFnMap = std::unordered_map<Identifier, FnObj>;

/** @brief Built-in function callable signature. */
using BuiltinFn = std::function<double(std::span<const double>)>;

/** @brief Metadata for built-in functions. */
struct BuiltinSpec {
    Identifier name;
    std::size_t arity;
    std::string description;
    BuiltinFn fn;
};

/** @brief Built-in function registry. */
using BuiltinMap = std::unordered_map<Identifier, BuiltinSpec>;

/** @brief Constant registry. */
using ConstantMap = std::unordered_map<Identifier, double>;

/** @brief REPL evaluation state. */
struct State {
    VariableMap vars;
    UserFnMap fns;
    double last_result = 0.0;
    bool has_last_result = false;
};

/** @brief Stream printer for variable maps. */
std::ostream& operator<<(std::ostream& os, const VariableMap& vars);

/** @brief Access built-in function registry. */
const BuiltinMap& builtin_functions();

/** @brief Access built-in constants registry. */
const ConstantMap& constants();

/** @brief Whether a name is reserved from assignment. */
bool is_reserved_identifier(std::string_view name);

/** @brief Whether a name matches a built-in function. */
bool is_builtin_function(std::string_view name);

/** @brief Whether a name matches a constant. */
bool is_constant(std::string_view name);

}  // namespace repl
