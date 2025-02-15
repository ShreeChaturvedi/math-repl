#include "repl/state.hpp"

#include <cmath>
#include <numbers>
#include <utility>

namespace repl {

std::ostream& operator<<(std::ostream& os, const VariableMap& vars) {
    if (vars.empty()) {
        return os << "{}";
    }
    os << '{';
    auto it = vars.begin();
    os << it->first << ": " << it->second;
    ++it;
    for (; it != vars.end(); ++it) {
        os << ", " << it->first << ": " << it->second;
    }
    return os << '}';
}

namespace {

BuiltinSpec make_unary(std::string name, std::string description,
                       double (*fn)(double)) {
    return BuiltinSpec{std::move(name), 1, std::move(description),
                       [fn](std::span<const double> args) { return fn(args[0]); }};
}

BuiltinSpec make_binary(std::string name, std::string description,
                        double (*fn)(double, double)) {
    return BuiltinSpec{std::move(name), 2, std::move(description),
                       [fn](std::span<const double> args) { return fn(args[0], args[1]); }};
}

}  // namespace

const BuiltinMap& builtin_functions() {
    static const BuiltinMap builtins = [] {
        BuiltinMap map;

        auto add = [&map](BuiltinSpec spec) {
            map.emplace(spec.name, std::move(spec));
        };

        add(make_unary("sin", "Sine (radians)", std::sin));
        add(make_unary("cos", "Cosine (radians)", std::cos));
        add(make_unary("tan", "Tangent (radians)", std::tan));
        add(make_unary("asin", "Inverse sine", std::asin));
        add(make_unary("acos", "Inverse cosine", std::acos));
        add(make_unary("atan", "Inverse tangent", std::atan));

        add(make_unary("sinh", "Hyperbolic sine", std::sinh));
        add(make_unary("cosh", "Hyperbolic cosine", std::cosh));
        add(make_unary("tanh", "Hyperbolic tangent", std::tanh));
        add(make_unary("asinh", "Inverse hyperbolic sine", std::asinh));
        add(make_unary("acosh", "Inverse hyperbolic cosine", std::acosh));
        add(make_unary("atanh", "Inverse hyperbolic tangent", std::atanh));

        add(make_unary("sqrt", "Square root", std::sqrt));
        add(make_unary("cbrt", "Cube root", std::cbrt));
        add(make_unary("exp", "Exponential (e^x)", std::exp));
        add(make_unary("ln", "Natural logarithm", std::log));
        add(make_unary("log", "Base-10 logarithm", std::log10));
        add(make_unary("log2", "Base-2 logarithm", std::log2));
        add(make_unary("abs", "Absolute value", std::fabs));
        add(make_unary("floor", "Round down", std::floor));
        add(make_unary("ceil", "Round up", std::ceil));
        add(make_unary("round", "Round to nearest", std::round));
        add(make_unary("trunc", "Truncate fractional part", std::trunc));

        add(make_binary("pow", "Power", std::pow));
        add(make_binary("fmod", "Floating-point modulo", std::fmod));
        add(make_binary("atan2", "Quadrant-aware arctangent", std::atan2));

        return map;
    }();

    return builtins;
}

const ConstantMap& constants() {
    static const ConstantMap values = {
        {"pi", std::numbers::pi_v<double>},
        {"e", std::numbers::e_v<double>},
        {"tau", std::numbers::pi_v<double> * 2.0},
    };
    return values;
}

bool is_reserved_identifier(std::string_view name) {
    if (name == "_") {
        return true;
    }
    return is_constant(name) || is_builtin_function(name);
}

bool is_builtin_function(std::string_view name) {
    const auto& builtins = builtin_functions();
    return builtins.find(std::string{name}) != builtins.end();
}

bool is_constant(std::string_view name) {
    const auto& values = constants();
    return values.find(std::string{name}) != values.end();
}

}  // namespace repl
