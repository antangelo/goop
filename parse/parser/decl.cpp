#include "decl.h"
#include "tokens.h"
#include <optional>

namespace goop {

namespace parse {

std::optional<ConstSpec> parse_const_spec(tokens::TokenStream &ts)
{
    auto id = parse_identifier_list(ts);
    if (!id) {
        return std::nullopt;
    }

    std::optional<Type> ty{std::nullopt};
    if (!ts.match_punctuation(tokens::Punctuation::Kind::ASSIGNMENT)) {
        if (ts.peek_punctuation(tokens::Punctuation::Kind::SEMICOLON, tokens::Punctuation::Kind::RPAREN)) {
            return ConstSpec(std::move(*id));
        }

        ty = parse_type(ts);
        if (!ty || !ts.match_punctuation(tokens::Punctuation::Kind::ASSIGNMENT)) {
            return std::nullopt;
        }
    }

    auto expr_list = parse_expression_list(ts);
    return ConstSpec(std::move(*id), std::move(ty), std::move(expr_list));
}

std::optional<ConstDecl> parse_const_decl(tokens::TokenStream &ts)
{
    if (!ts.match_keyword(tokens::Keyword::Kind::CONST)) {
        return std::nullopt;
    }

    std::vector<ConstSpec> specs;

    if (ts.match_punctuation(tokens::Punctuation::Kind::LPAREN)) {
        while (auto spec = parse_const_spec(ts)) {
            specs.push_back(std::move(*spec));

            if (ts.peek_punctuation(tokens::Punctuation::Kind::RPAREN)) {
                break;
            }

            if (!ts.match_punctuation(tokens::Punctuation::Kind::SEMICOLON)) {
                break;
            }
        }

        if (!ts.match_punctuation(tokens::Punctuation::Kind::RPAREN)) {
            return std::nullopt;
        }
    } else {
        auto spec = parse_const_spec(ts);
        if (!spec) {
            return std::nullopt;
        }

        specs.push_back(std::move(*spec));
    }

    return ConstDecl(std::move(specs));
}

std::optional<VarSpec> parse_var_spec(tokens::TokenStream &ts)
{
    auto id = parse_identifier_list(ts);
    if (!id) {
        return std::nullopt;
    }

    std::optional<Type> ty{std::nullopt};
    if (!ts.match_punctuation(tokens::Punctuation::Kind::ASSIGNMENT)) {
        ty = parse_type(ts);
        if (!ty) {
            return std::nullopt;
        }

        if (!ts.match_punctuation(tokens::Punctuation::Kind::ASSIGNMENT)) {
            return VarSpec(std::move(*id), std::move(ty), std::nullopt);
        }
    }

    auto expr_list = parse_expression_list(ts);
    if (!ty && expr_list.is_empty()) {
        return std::nullopt;
    }

    return VarSpec(std::move(*id), std::move(ty), std::move(expr_list));
}

std::optional<VarDecl> parse_var_decl(tokens::TokenStream &ts)
{
    if (!ts.match_keyword(tokens::Keyword::Kind::VAR)) {
        return std::nullopt;
    }

    std::vector<VarSpec> specs;

    if (ts.match_punctuation(tokens::Punctuation::Kind::LPAREN)) {
        while (auto spec = parse_var_spec(ts)) {
            specs.push_back(std::move(*spec));

            if (ts.peek_punctuation(tokens::Punctuation::Kind::RPAREN)) {
                break;
            }

            if (!ts.match_punctuation(tokens::Punctuation::Kind::SEMICOLON)) {
                break;
            }
        }

        if (!ts.match_punctuation(tokens::Punctuation::Kind::RPAREN)) {
            return std::nullopt;
        }
    } else {
        auto spec = parse_var_spec(ts);
        if (!spec) {
            return std::nullopt;
        }

        specs.push_back(std::move(*spec));
    }

    return VarDecl(std::move(specs));
}

} // namespace parse

} // namespace goop
