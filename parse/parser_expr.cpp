#include "parser_expr.h"
#include "parser_common.h"
#include "parser_type.h"
#include "tokens.h"
#include <cassert>
#include <memory>
#include <optional>
#include <vector>

namespace goop {

namespace parse {

int binary_binding_power(tokens::Punctuation::Kind kind)
{
    using PK = tokens::Punctuation::Kind;

    switch (kind) {
        case PK::STAR:
        case PK::SLASH:
        case PK::PERCENT:
        case PK::LSHIFT:
        case PK::RSHIFT:
        case PK::AMP:
        case PK::BITCLEAR:
            return 5;

        case PK::PLUS:
        case PK::MINUS:
        case PK::PIPE:
        case PK::CARAT:
            return 4;

        case PK::EQUAL:
        case PK::NOT_EQUAL:
        case PK::LESS_THAN:
        case PK::LESS_THAN_EQUAL:
        case PK::GREATER_THAN:
        case PK::GREATER_THAN_EQUAL:
            return 3;

        case PK::BOOL_AND:
            return 2;

        case PK::BOOL_OR:
            return 1;

        default:
            assert(!"Invalid operator given to binary_binding_power");
    }
}

std::optional<tokens::Punctuation> match_binary_op(tokens::TokenStream &ts)
{
    using PK = tokens::Punctuation::Kind;

    return ts.match_punctuation(
        PK::STAR,
        PK::SLASH,
        PK::PERCENT,
        PK::LSHIFT,
        PK::RSHIFT,
        PK::AMP,
        PK::BITCLEAR,
        PK::PLUS,
        PK::MINUS,
        PK::PIPE,
        PK::CARAT,
        PK::EQUAL,
        PK::NOT_EQUAL,
        PK::LESS_THAN,
        PK::LESS_THAN_EQUAL,
        PK::GREATER_THAN,
        PK::GREATER_THAN_EQUAL,
        PK::BOOL_AND,
        PK::BOOL_OR);
}

std::optional<tokens::Punctuation> match_unary_op(tokens::TokenStream &ts)
{
    using PK = tokens::Punctuation::Kind;
    return ts.match_punctuation(PK::PLUS, PK::MINUS, PK::BANG, PK::CARAT, PK::STAR, PK::AMP, PK::RECEIVE);
}

std::unique_ptr<Expression> parse_expression_pratt(tokens::TokenStream &ts, int binding_power)
{
    auto lhs_unary = parse_unary_expression(ts);
    if (!lhs_unary) {
        return nullptr;
    }

    auto lhs = std::unique_ptr<Expression>(new UnaryExpression(std::move(*lhs_unary)));

    while (true) {
        auto op = match_binary_op(ts);
        if (!op) {
            break;
        }

        int left_binding_power = 2 * binary_binding_power(op->kind);
        int right_binding_power = left_binding_power + 1;
        if (left_binding_power < binding_power) {
            ts.unget(*op);
            break;
        }

        auto rhs = parse_expression_pratt(ts, right_binding_power);

        lhs = std::unique_ptr<Expression>(new BinaryExpression(*op, std::move(lhs), std::move(rhs)));
    }

    return lhs;
}

std::unique_ptr<Expression> parse_expression(tokens::TokenStream &ts)
{
    return parse_expression_pratt(ts, 0);
}

std::optional<UnaryExpression> parse_unary_expression(tokens::TokenStream &ts)
{
    using PK = tokens::Punctuation::Kind;
    
    std::vector<tokens::Punctuation> unary_ops;
    while (auto unary_op =
            ts.match_punctuation(PK::PLUS, PK::MINUS, PK::BANG, PK::CARAT,
                                 PK::STAR, PK::AMP, PK::RECEIVE)) {
        unary_ops.push_back(*unary_op);
    }

    if (auto primary = parse_primary_expression(ts)) {
        return UnaryExpression(std::move(*primary), std::move(unary_ops));
    }

    return std::nullopt;
}

std::optional<PrimaryExpression::Inner> parse_pex_inner(tokens::TokenStream &ts)
{
    if (auto ident = parse_ident_or_qualified(ts)) {
        if (auto type_args = parse_type_args(ts)) {
            return NamedOperand(std::move(*ident), std::move(*type_args));
        }

        return ident;
    }

    if (ts.match_punctuation(tokens::Punctuation::Kind::LPAREN)) {
        auto exp = parse_expression(ts);
        if (!exp || !ts.match_punctuation(tokens::Punctuation::Kind::RPAREN)) {
            return std::nullopt;
        }

        return ParenExpression(std::move(exp));
    }

#define MATCH_BASIC_LITERAL(ty) \
    if (auto lit = ts.match_consume<ty>()) { \
        return BasicLiteral(*lit); \
    }

    MATCH_BASIC_LITERAL(tokens::IntLiteral)
    MATCH_BASIC_LITERAL(tokens::FloatLiteral)
    MATCH_BASIC_LITERAL(tokens::ImaginaryLiteral)
    MATCH_BASIC_LITERAL(tokens::RuneLiteral)
    MATCH_BASIC_LITERAL(tokens::StringLiteral)

#undef MATCH_BASIC_LITERAL

    // FIXME: Composite Literals
    // FIXME: Function Literals

    return std::nullopt;
}

std::optional<Slice> finish_parsing_slice(tokens::TokenStream &ts, std::unique_ptr<Expression> low)
{
    auto high = parse_expression(ts);
    bool rbracket = bool(ts.match_punctuation(tokens::Punctuation::Kind::RBRACKET));
    if (!high && rbracket) {
        return Slice();
    }

    if (high && rbracket) {
        return Slice(std::move(low), std::move(high));
    }

    if (!high || !ts.match_punctuation(tokens::Punctuation::Kind::COLON)) {
        return std::nullopt;
    }

    auto max = parse_expression(ts);
    if (!max || !ts.match_punctuation(tokens::Punctuation::Kind::RBRACKET)) {
        return std::nullopt;
    }

    return Slice(std::move(low), std::move(high), std::move(max));
}

std::optional<PrimaryExpression::Outer> parse_pex_outer(tokens::TokenStream &ts)
{
    if (ts.match_punctuation(tokens::Punctuation::Kind::DOT)) {
        if (ts.match_punctuation(tokens::Punctuation::Kind::LPAREN)) {
            auto type = parse_type(ts);
            if (!type || !ts.match_punctuation(tokens::Punctuation::Kind::RPAREN)) {
                return std::nullopt;
            }

            return TypeAssertion(std::make_unique<Type>(std::move(*type)));
        }

        auto ident = ts.match_consume<tokens::Identifier>();
        if (!ident) {
            return std::nullopt;
        }

        return Selector(std::move(*ident));
    }

    if (ts.match_punctuation(tokens::Punctuation::Kind::LBRACKET)) {
        if (ts.match_punctuation(tokens::Punctuation::Kind::COLON)) {
            return finish_parsing_slice(ts, nullptr);
        }

        auto exp = parse_expression(ts);
        if (!exp) {
            return std::nullopt;
        }

        if (ts.match_punctuation(tokens::Punctuation::Kind::COLON)) {
            return finish_parsing_slice(ts, std::move(exp));
        }

        ts.match_punctuation(tokens::Punctuation::Kind::COMMA);

        if (!ts.match_punctuation(tokens::Punctuation::Kind::RBRACKET)) {
            return std::nullopt;
        }

        return Index(std::move(exp));
    }

    if (ts.match_punctuation(tokens::Punctuation::Kind::LPAREN)) {
        auto expr_list = parse_expression_list(ts);
        bool elipses = bool(ts.match_punctuation(tokens::Punctuation::Kind::ELIPSES));

        ts.match_punctuation(tokens::Punctuation::Kind::ELIPSES);

        if (!ts.match_punctuation(tokens::Punctuation::Kind::RPAREN)) {
            return std::nullopt;
        }

        return Arguments(std::move(expr_list), elipses);
    }

    return std::nullopt;
}

std::optional<PrimaryExpression>
parse_primary_expression(tokens::TokenStream &ts)
{
    auto inner = parse_pex_inner(ts);
    if (!inner) {
        return std::nullopt;
    }

    std::vector<PrimaryExpression::Outer> outers;
    while (auto outer = parse_pex_outer(ts)) {
        outers.push_back(std::move(*outer));
    }

    return PrimaryExpression(std::move(*inner), std::move(outers));
}

ExpressionList parse_expression_list(tokens::TokenStream &ts)
{
    std::vector<std::unique_ptr<Expression>> exps;

    while (auto exp = parse_expression(ts)) {
        exps.push_back(std::move(exp));

        if (!ts.match_punctuation(tokens::Punctuation::Kind::COMMA)) {
            break;
        }
    }

    return ExpressionList(std::move(exps));
}

bool ExpressionList::is_empty() const
{
    return exps.empty();
}

} // namespace parse

} // namespace goop
