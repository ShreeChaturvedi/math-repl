#include "repl/expression.hpp"

#include <format>
#include <functional>

namespace repl {

std::ostream& operator<<(std::ostream& os, EType type) {
    switch (type) {
        case EType::Number: return os << "Number";
        case EType::Variable: return os << "Variable";
        case EType::Unary: return os << "Unary Expression";
        case EType::Binary: return os << "Binary Expression";
        case EType::FnCall: return os << "FunctionCall";
        case EType::Ternary: return os << "Ternary Expression";
    }
    return os << "Unknown";
}

ExpressionPtr make_number(double value) {
    auto expr = std::make_unique<Expression>();
    expr->type = EType::Number;
    expr->data = value;
    return expr;
}

ExpressionPtr make_variable(Identifier name) {
    auto expr = std::make_unique<Expression>();
    expr->type = EType::Variable;
    expr->data = std::move(name);
    return expr;
}

ExpressionPtr make_unary(TType op, ExpressionPtr right) {
    auto expr = std::make_unique<Expression>();
    expr->type = EType::Unary;
    expr->data = UnaryNode{.op = op, .right = std::move(right)};
    return expr;
}

ExpressionPtr make_binary(TType op, ExpressionPtr left, ExpressionPtr right) {
    auto expr = std::make_unique<Expression>();
    expr->type = EType::Binary;
    expr->data = BinaryNode{.op = op, .left = std::move(left), .right = std::move(right)};
    return expr;
}

ExpressionPtr make_fn_call(Identifier name, ExpressionList args) {
    auto expr = std::make_unique<Expression>();
    expr->type = EType::FnCall;
    expr->data = FnNode{.name = std::move(name), .args = std::move(args)};
    return expr;
}

ExpressionPtr make_ternary(ExpressionPtr condition, ExpressionPtr then_branch,
                           ExpressionPtr else_branch) {
    auto expr = std::make_unique<Expression>();
    expr->type = EType::Ternary;
    expr->data = TernaryNode{.condition = std::move(condition),
                             .then_branch = std::move(then_branch),
                             .else_branch = std::move(else_branch)};
    return expr;
}

namespace {

ExpressionPtr parse_assignment(TokenStream& stream);

ExpressionList parse_fn_args(TokenStream& stream) {
    stream.expect(TType::LParen);

    std::vector<Tokens> token_sets;
    token_sets.emplace_back();
    int paren_depth = 0;

    while (!stream.empty()) {
        if (stream.peek().type == TType::RParen && paren_depth == 0) {
            break;
        }

        const Token& token = stream.get();
        if (token.type == TType::LParen) {
            ++paren_depth;
        } else if (token.type == TType::RParen) {
            if (paren_depth == 0) {
                throw ParseError("Unexpected ')' inside function arguments");
            }
            --paren_depth;
        }

        if (token.type == TType::Comma && paren_depth == 0) {
            token_sets.emplace_back();
            continue;
        }
        token_sets.back().push_back(token);
    }

    if (stream.empty()) {
        throw ParseError("Expected ')' to close function call");
    }
    stream.get();

    if (token_sets.size() == 1 && token_sets.front().empty()) {
        return {};
    }

    ExpressionList args;
    args.reserve(token_sets.size());
    for (const auto& tokens : token_sets) {
        if (tokens.empty()) {
            throw ParseError("Empty function argument");
        }
        args.push_back(parse(tokens));
    }

    return args;
}

ExpressionPtr parse_primary(TokenStream& stream) {
    if (stream.empty()) {
        throw ParseError("Unexpected end of input while parsing expression");
    }

    const Token& current = stream.get();
    switch (current.type) {
        case TType::Number:
            return make_number(current.get<double>());
        case TType::Identifier: {
            Identifier id = current.get<Identifier>();
            if (!stream.empty() && stream.peek().type == TType::LParen) {
                return make_fn_call(std::move(id), parse_fn_args(stream));
            }
            return make_variable(std::move(id));
        }
        case TType::LParen: {
            ExpressionPtr result = parse_assignment(stream);
            if (stream.empty()) {
                throw ParseError("Expected ')' to close expression");
            }
            const Token& closing = stream.get();
            if (closing.type != TType::RParen) {
                throw ParseError(std::format("Expected ')' but found {}",
                                             to_string(closing.type)));
            }
            return result;
        }
        default:
            throw ParseError(std::format(
                "Could not parse expression starting with token '{}'", to_string(current.type)));
    }
}

ExpressionPtr parse_unary(TokenStream& stream) {
    if (stream.empty()) {
        throw ParseError("Unexpected end of input while parsing unary expression");
    }

    if (stream.peek().type == TType::Plus || stream.peek().type == TType::Minus) {
        TType op = stream.get().type;
        return make_unary(op, parse_unary(stream));
    }

    return parse_primary(stream);
}

using TTypes = std::vector<TType>;

enum class Associativity { Left, Right };

ExpressionPtr abstract_parse(TokenStream& stream,
                             const std::function<ExpressionPtr(TokenStream&)>& parser,
                             const std::function<bool(const Token&)>& is_operator,
                             Associativity assoc,
                             ExpressionPtr left = nullptr) {
    if (!left) {
        left = parser(stream);
    }

    while (!stream.empty() && is_operator(stream.peek())) {
        TType op = stream.get().type;
        if (assoc == Associativity::Right) {
            ExpressionPtr right = abstract_parse(stream, parser, is_operator, assoc, nullptr);
            return make_binary(op, std::move(left), std::move(right));
        }

        ExpressionPtr right = parser(stream);
        left = make_binary(op, std::move(left), std::move(right));
    }

    return left;
}

ExpressionPtr abstract_parse(TokenStream& stream,
                             const std::function<ExpressionPtr(TokenStream&)>& parser,
                             const TTypes& operators,
                             Associativity assoc = Associativity::Left) {
    auto valid_operator = [&operators](const Token& token) {
        for (const auto& op : operators) {
            if (token.type == op) {
                return true;
            }
        }
        return false;
    };
    return abstract_parse(stream, parser, valid_operator, assoc, nullptr);
}

ExpressionPtr parse_power(TokenStream& stream) {
    return abstract_parse(stream, parse_unary, {TType::Caret}, Associativity::Right);
}

ExpressionPtr parse_term(TokenStream& stream) {
    return abstract_parse(stream, parse_power, {TType::Star, TType::Slash, TType::Percent});
}

ExpressionPtr parse_additive(TokenStream& stream) {
    return abstract_parse(stream, parse_term, {TType::Plus, TType::Minus});
}

ExpressionPtr parse_relational(TokenStream& stream) {
    return abstract_parse(stream, parse_additive,
                          {TType::Less, TType::LessEqual, TType::Greater, TType::GreaterEqual});
}

ExpressionPtr parse_equality(TokenStream& stream) {
    return abstract_parse(stream, parse_relational, {TType::EqualEqual, TType::BangEqual});
}

ExpressionPtr parse_ternary(TokenStream& stream) {
    ExpressionPtr condition = parse_equality(stream);
    if (!stream.empty() && stream.peek().type == TType::Question) {
        stream.get();
        ExpressionPtr then_branch = parse_ternary(stream);
        if (stream.empty() || stream.get().type != TType::Colon) {
            throw ParseError("Expected ':' in ternary expression");
        }
        ExpressionPtr else_branch = parse_ternary(stream);
        return make_ternary(std::move(condition), std::move(then_branch),
                            std::move(else_branch));
    }
    return condition;
}

ExpressionPtr parse_assignment(TokenStream& stream) {
    return abstract_parse(stream, parse_ternary, {TType::Equals}, Associativity::Right);
}

}  // namespace

ExpressionPtr parse(TokenStream& stream) {
    return parse_assignment(stream);
}

ExpressionPtr parse(const Tokens& tokens) {
    TokenStream stream{tokens};
    ExpressionPtr expr = parse(stream);
    if (!stream.empty()) {
        throw ParseError(std::format("Unexpected token '{}'", to_string(stream.peek().type)));
    }
    return expr;
}

}  // namespace repl
