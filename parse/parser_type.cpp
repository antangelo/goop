#include "parser.h"
#include "parser_type.h"
#include "tokens.h"
#include "util.h"
#include <optional>

namespace goop
{

namespace parse
{

std::optional<TypeName> parse_type_name(tokens::TokenStream &ts)
{
    auto name_or_package = ts.match_consume<tokens::Identifier>();
    if (!name_or_package) {
        return std::nullopt;
    }

    if (ts.match_punctuation(tokens::Punctuation::Kind::DOT)) {
        auto name = ts.match_consume<tokens::Identifier>();
        if (!name) {
            return std::nullopt;
        }

        return TypeName(*name_or_package, *name);
    }

    return TypeName(*name_or_package);
}

std::optional<box<TypeList>> parse_type_list(tokens::TokenStream &ts)
{
    std::vector<Type> type_list;

    do {
        auto type = parse_type(ts);
        if (!type) {
            break;
        }

        type_list.push_back(*type);
    } while (ts.match_punctuation(tokens::Punctuation::Kind::COMMA));

    if (type_list.empty())
        return std::nullopt;

    return make_box<TypeList>(type_list);
}

std::optional<box<TypeList>> parse_type_args(tokens::TokenStream &ts)
{
    if (!ts.match_punctuation(tokens::Punctuation::Kind::LBRACKET)) {
        return std::nullopt;
    }

    auto tl = parse_type_list(ts);
    if (tl && ts.match_punctuation(tokens::Punctuation::Kind::RBRACKET)) {
        return tl;
    }

    return std::nullopt;
}

std::optional<Type> parse_type(tokens::TokenStream &ts)
{
    if (auto type_name = parse_type_name(ts)) {
        if (auto type_args = parse_type_args(ts)) {
            return Type(Type::NamedType(*type_name, std::move(*type_args)));
        }

        return Type(Type::NamedType(*type_name));
    }

    if (auto lit = parse_type_lit(ts)) {
        return Type(*lit);
    }

    if (ts.match_punctuation(tokens::Punctuation::Kind::LPAREN)) {
        auto type = parse_type(ts);
        if (type && ts.match_punctuation(tokens::Punctuation::Kind::RPAREN)) {
            return type;
        }
    }

    return std::nullopt;
}

std::optional<TypeLit> parse_type_lit(tokens::TokenStream &ts)
{
    return std::nullopt;
}

std::optional<ChannelType> parse_channel_type(tokens::TokenStream &ts)
{
    if (ts.match_keyword(tokens::Keyword::Kind::CHAN)) {
        auto recv = ts.match_punctuation(tokens::Punctuation::RECEIVE);
        auto type = parse_type(ts);

        if (!type) {
            return std::nullopt;
        }

        return ChannelType(
                recv ? ChannelType::Direction::RECV : ChannelType::Direction::BIDI,
                std::make_unique<Type>(*type)
                );
    }

    if (ts.match_punctuation(tokens::Punctuation::Kind::RECEIVE)) {
        if (!ts.match_keyword(tokens::Keyword::Kind::CHAN)) {
            return std::nullopt;
        }
    }

    return std::nullopt;
}

}

}
