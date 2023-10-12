#include "statement.h"
#include "parser/expr.h"
#include "tokens.h"
#include <memory>

namespace goop {

namespace parse {

std::unique_ptr<Statement> parse_statement(tokens::TokenStream &ts)
{
    if (auto ident = ts.match_consume<tokens::Identifier>()) {
        if (ts.match_punctuation(tokens::PunctuationKind::COLON)) {
            auto stmt = parse_statement(ts);
            return std::make_unique<LabeledStatement>(std::move(*ident), std::move(stmt));
        }

        ts.unget(*ident);
    }

    // Add unambiguous ones like "if", "go", etc. here
    // They are faster to rule out

    using KW = tokens::KeywordKind;

    if (ts.match_keyword(KW::GO)) {
        auto expr = parse_expression(ts);
        if (!expr) {
            return nullptr;
        }

        return std::make_unique<GoStatement>(std::move(expr));
    }

    if (ts.match_keyword(KW::RETURN)) {
        auto values = parse_expression_list(ts);
        return std::make_unique<ReturnStatement>(std::move(values));
    }

    if (ts.match_keyword(KW::BREAK)) {
        auto label = ts.match_consume<tokens::Identifier>();
        return std::make_unique<BreakStatement>(*label);
    }

    if (ts.match_keyword(KW::CONTINUE)) {
        auto label = ts.match_consume<tokens::Identifier>();
        return std::make_unique<ContinueStatement>(*label);
    }

    if (ts.match_keyword(KW::GOTO)) {
        auto label = ts.match_consume<tokens::Identifier>();
        if (!label) {
            return nullptr;
        }

        return std::make_unique<GotoStatement>(*label);
    }

    if (ts.match_keyword(KW::FALLTHROUGH)) {
        return std::make_unique<FallthroughStatement>();
    }

    if (ts.match_keyword(KW::IF)) {
    }

    if (ts.match_keyword(KW::SWITCH)) {
    }

    if (ts.match_keyword(KW::SELECT)) {
    }

    if (ts.match_keyword(KW::FOR)) {
    }

    if (ts.match_keyword(KW::DEFER)) {
        auto expr = parse_expression(ts);
        if (!expr) {
            return nullptr;
        }

        return std::make_unique<DeferStatement>(std::move(expr));
    }

    if (auto block = parse_block(ts)) {
        return block;
    }

    return parse_simple_statement(ts);
}

std::unique_ptr<SimpleStatement> parse_simple_statement(tokens::TokenStream &ts)
{
    if (ts.peek_punctuation(tokens::PunctuationKind::SEMICOLON)) {
        return std::make_unique<EmptyStatement>();
    }

    auto expr_list = parse_expression_list(ts);
    auto assign_op = ts.match_punctuation(
            tokens::PunctuationKind::PLUS_EQUAL,
            tokens::PunctuationKind::MINUS_EQUAL,
            tokens::PunctuationKind::OR_EQUAL,
            tokens::PunctuationKind::XOR_EQUAL,
            tokens::PunctuationKind::STAR_EQUAL,
            tokens::PunctuationKind::SLASH_EQUAL,
            tokens::PunctuationKind::MOD_EQUAL,
            tokens::PunctuationKind::LSHIFT_EQUAL,
            tokens::PunctuationKind::RSHIFT_EQUAL,
            tokens::PunctuationKind::AND_EQUAL,
            tokens::PunctuationKind::BITCLEAR_EQUAL
            );
    if (assign_op) {
        auto assignment = parse_expression_list(ts);
        return std::make_unique<AssignmentStatement>(std::move(expr_list), std::move(assignment), *assign_op);
    }

    if (expr_list.size() == 1) {
        if (ts.match_punctuation(tokens::PunctuationKind::RECEIVE)) {
            if (auto expr = parse_expression(ts)) {
                return std::make_unique<SendStatement>(std::move(expr_list[0]), std::move(expr));
            }

            // expected-expression
            return nullptr;
        }

        if (auto inc_dec = ts.match_punctuation(tokens::PunctuationKind::INCREMENT, tokens::PunctuationKind::DECREMENT)) {
            return std::make_unique<IncrDecrStatement>(std::move(expr_list[0]), inc_dec->kind == tokens::PunctuationKind::INCREMENT);
        }

        return std::make_unique<ExpressionStatement>(std::move(expr_list[0]));
    }

    return nullptr;
}

}

}
