#pragma once

#include <optional>
#include <string>
#include <string_view>

#include "repl/expression.hpp"
#include "repl/state.hpp"

namespace repl {

/** @brief Evaluation result for a top-level query. */
struct EvalResult {
    std::optional<double> value;
    std::optional<std::string> info;
};

/** @brief Evaluate a parsed expression in the given state.
 *  @throws EvalError on invalid evaluation.
 */
EvalResult evaluate(Expression& expr, State& state);

/** @brief Parse and evaluate a source string.
 *  @throws ParseError or EvalError on failure.
 */
EvalResult process_query(std::string_view input, State& state);

}  // namespace repl
