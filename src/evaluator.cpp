#include "repl/evaluator.hpp"

#include <cmath>
#include <format>
#include <unordered_set>

namespace repl {

namespace {

struct EvalContext {
    VariableMap* locals = nullptr;
    bool allow_function_definition = false;
};

std::string join_params(const Identifiers& params) {
    std::string result;
    for (std::size_t index = 0; index < params.size(); ++index) {
        if (index > 0) {
            result += ", ";
        }
        result += params[index];
    }
    return result;
}

double eval_value(Expression& expr, State& state, EvalContext& ctx);

double eval_binary(BinaryNode& node, State& state, EvalContext& ctx) {
    switch (node.op) {
        case TType::Plus:
            return eval_value(*node.left, state, ctx) + eval_value(*node.right, state, ctx);
        case TType::Minus:
            return eval_value(*node.left, state, ctx) - eval_value(*node.right, state, ctx);
        case TType::Star:
            return eval_value(*node.left, state, ctx) * eval_value(*node.right, state, ctx);
        case TType::Slash: {
            double rhs = eval_value(*node.right, state, ctx);
            if (rhs == 0.0) {
                throw EvalError("Division by zero");
            }
            return eval_value(*node.left, state, ctx) / rhs;
        }
        case TType::Percent: {
            double rhs = eval_value(*node.right, state, ctx);
            if (rhs == 0.0) {
                throw EvalError("Modulo by zero");
            }
            return std::fmod(eval_value(*node.left, state, ctx), rhs);
        }
        case TType::Caret:
            return std::pow(eval_value(*node.left, state, ctx),
                            eval_value(*node.right, state, ctx));
        case TType::Less:
            return eval_value(*node.left, state, ctx) < eval_value(*node.right, state, ctx);
        case TType::LessEqual:
            return eval_value(*node.left, state, ctx) <= eval_value(*node.right, state, ctx);
        case TType::Greater:
            return eval_value(*node.left, state, ctx) > eval_value(*node.right, state, ctx);
        case TType::GreaterEqual:
            return eval_value(*node.left, state, ctx) >= eval_value(*node.right, state, ctx);
        case TType::EqualEqual:
            return eval_value(*node.left, state, ctx) == eval_value(*node.right, state, ctx);
        case TType::BangEqual:
            return eval_value(*node.left, state, ctx) != eval_value(*node.right, state, ctx);
        case TType::Equals: {
            if (node.left->type != EType::Variable) {
                throw EvalError("Left side of '=' must be a variable name");
            }
            const auto& name = node.left->get<Identifier>();
            if (is_reserved_identifier(name)) {
                throw EvalError(std::format("'{}' is read-only", name));
            }
            double value = eval_value(*node.right, state, ctx);
            if (ctx.locals) {
                (*ctx.locals)[name] = value;
            } else {
                state.vars[name] = value;
            }
            return value;
        }
        default:
            throw EvalError("Invalid or unsupported operator type");
    }
}

double eval_function_call(FnNode& node, State& state, EvalContext& ctx) {
    const auto& builtins = builtin_functions();
    if (auto it = builtins.find(node.name); it != builtins.end()) {
        const BuiltinSpec& spec = it->second;
        if (node.args.size() != spec.arity) {
            throw EvalError(std::format("Function '{}' expects {} arguments, got {}",
                                        node.name, spec.arity, node.args.size()));
        }
        std::vector<double> args;
        args.reserve(node.args.size());
        for (const auto& arg : node.args) {
            args.push_back(eval_value(*arg, state, ctx));
        }
        return spec.fn(args);
    }

    auto it = state.fns.find(node.name);
    if (it == state.fns.end()) {
        throw EvalError(std::format("Function '{}' not defined", node.name));
    }

    const FnObj& fn_obj = it->second;
    if (node.args.size() != fn_obj.params.size()) {
        throw EvalError(std::format("Function '{}' expects {} arguments, got {}",
                                    node.name, fn_obj.params.size(), node.args.size()));
    }

    VariableMap locals;
    locals.reserve(fn_obj.params.size());
    for (std::size_t index = 0; index < fn_obj.params.size(); ++index) {
        locals[fn_obj.params[index]] = eval_value(*node.args[index], state, ctx);
    }

    EvalContext local_ctx{&locals, false};
    return eval_value(*fn_obj.expr, state, local_ctx);
}

double eval_ternary(TernaryNode& node, State& state, EvalContext& ctx) {
    double condition = eval_value(*node.condition, state, ctx);
    if (condition != 0.0) {
        return eval_value(*node.then_branch, state, ctx);
    }
    return eval_value(*node.else_branch, state, ctx);
}

double eval_value(Expression& expr, State& state, EvalContext& ctx) {
    switch (expr.type) {
        case EType::Number:
            return expr.get<double>();
        case EType::Variable: {
            const auto& name = expr.get<Identifier>();
            if (ctx.locals) {
                auto it = ctx.locals->find(name);
                if (it != ctx.locals->end()) {
                    return it->second;
                }
            }
            if (name == "_") {
                if (!state.has_last_result) {
                    throw EvalError("No previous result available for '_'");
                }
                return state.last_result;
            }
            auto var_it = state.vars.find(name);
            if (var_it != state.vars.end()) {
                return var_it->second;
            }
            const auto& values = constants();
            if (auto const_it = values.find(name); const_it != values.end()) {
                return const_it->second;
            }
            throw EvalError(std::format("Variable '{}' not defined", name));
        }
        case EType::Unary: {
            auto& node = expr.get<UnaryNode>();
            double value = eval_value(*node.right, state, ctx);
            return node.op == TType::Plus ? value : -value;
        }
        case EType::Binary:
            return eval_binary(expr.get<BinaryNode>(), state, ctx);
        case EType::FnCall:
            return eval_function_call(expr.get<FnNode>(), state, ctx);
        case EType::Ternary:
            return eval_ternary(expr.get<TernaryNode>(), state, ctx);
    }

    throw EvalError("Invalid expression type");
}

EvalResult define_function(BinaryNode& node, State& state) {
    if (node.left->type != EType::FnCall) {
        throw EvalError("Invalid function definition");
    }

    auto& fn_node = node.left->get<FnNode>();
    if (is_reserved_identifier(fn_node.name)) {
        throw EvalError(std::format("'{}' is read-only", fn_node.name));
    }

    Identifiers params;
    params.reserve(fn_node.args.size());
    std::unordered_set<Identifier> seen;

    for (const auto& arg : fn_node.args) {
        if (arg->type != EType::Variable) {
            throw EvalError("Function parameters must be identifiers");
        }
        const auto& name = arg->get<Identifier>();
        if (is_reserved_identifier(name)) {
            throw EvalError(std::format("'{}' is read-only", name));
        }
        if (!seen.insert(name).second) {
            throw EvalError(std::format("Duplicate parameter '{}'", name));
        }
        params.push_back(name);
    }

    if (!node.right) {
        throw EvalError("Function definition is missing a body");
    }

    state.fns[fn_node.name] = FnObj{params, std::move(node.right)};

    return EvalResult{std::nullopt,
                      std::format("Defined {}({})", fn_node.name, join_params(params))};
}

}  // namespace

EvalResult evaluate(Expression& expr, State& state) {
    EvalContext ctx{nullptr, true};

    if (expr.type == EType::Binary) {
        auto& node = expr.get<BinaryNode>();
        if (node.op == TType::Equals && node.left && node.left->type == EType::FnCall) {
            return define_function(node, state);
        }
    }

    double value = eval_value(expr, state, ctx);
    return EvalResult{value, std::nullopt};
}

EvalResult process_query(std::string_view input, State& state) {
    Tokens tokens = tokenize(input);
    ExpressionPtr expr = parse(tokens);
    EvalResult result = evaluate(*expr, state);
    if (result.value) {
        state.last_result = *result.value;
        state.has_last_result = true;
    }
    return result;
}

}  // namespace repl
