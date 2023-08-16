#include "parser_type.h"
#include "parser_expr.h"
#include "parser.h"
#include "parser_common.h"
#include "tokens.h"
#include <optional>

namespace goop {

namespace parse {

std::optional<std::unique_ptr<TypeList>>
parse_type_list(tokens::TokenStream &ts)
{
    std::vector<Type> type_list;

    do {
        auto type = parse_type(ts);
        if (!type) {
            break;
        }

        type_list.push_back(std::move(*type));
    } while (ts.match_punctuation(tokens::Punctuation::Kind::COMMA));

    if (type_list.empty())
        return std::nullopt;

    return std::make_unique<TypeList>(std::move(type_list));
}

std::optional<std::unique_ptr<TypeList>>
parse_type_args(tokens::TokenStream &ts)
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

std::optional<NamedType> parse_named_type(tokens::TokenStream &ts)
{
    if (auto type_name = parse_ident_or_qualified(ts)) {
        if (auto type_args = parse_type_args(ts)) {
            return NamedType(*type_name, std::move(*type_args));
        }

        return NamedType(*type_name);
    }

    return std::nullopt;
}

std::optional<Type> parse_type(tokens::TokenStream &ts)
{
    if (auto named_type = parse_named_type(ts)) {
        return Type(std::move(*named_type));
    }

    if (auto lit = parse_type_lit(ts)) {
        return Type(std::move(*lit));
    }

    if (ts.match_punctuation(tokens::Punctuation::Kind::LPAREN)) {
        auto type = parse_type(ts);
        if (type && ts.match_punctuation(tokens::Punctuation::Kind::RPAREN)) {
            return type;
        }
    }

    return std::nullopt;
}

std::optional<TypeDecl> parse_type_decl(tokens::TokenStream &ts)
{
    if (!ts.match_keyword(tokens::Keyword::Kind::TYPE)) {
        return std::nullopt;
    }

    std::vector<std::unique_ptr<TypeSpec>> types;
    if (ts.match_punctuation(tokens::Punctuation::Kind::LPAREN)) {
        do {
            if (auto alias = parse_alias_decl(ts)) {
                auto alias_ptr = std::make_unique<AliasDecl>(std::move(*alias));
                types.push_back(std::move(alias_ptr));
                continue;
            }

            if (auto td = parse_type_def(ts)) {
                auto td_ptr = std::make_unique<TypeDef>(std::move(*td));
                types.push_back(std::move(td_ptr));
                continue;
            }

            return std::nullopt;
        } while (ts.match_punctuation(tokens::Punctuation::Kind::SEMICOLON));

        if (!ts.match_punctuation(tokens::Punctuation::Kind::RPAREN)) {
            return std::nullopt;
        }

        return TypeDecl(std::move(types));
    }

    if (auto alias = parse_alias_decl(ts)) {
        auto alias_ptr = std::make_unique<AliasDecl>(std::move(*alias));
        types.push_back(std::move(alias_ptr));
    } else if (auto td = parse_type_def(ts)) {
        auto td_ptr = std::make_unique<TypeDef>(std::move(*td));
        types.push_back(std::move(td_ptr));
    } else {
        return std::nullopt;
    }

    return TypeDecl(std::move(types));
}

std::optional<TypeDef> parse_type_def(tokens::TokenStream &ts)
{
    // TODO
    (void)ts;
    return std::nullopt;
}

std::optional<AliasDecl> parse_alias_decl(tokens::TokenStream &ts)
{
    auto id = ts.match_consume<tokens::Identifier>();
    if (!id) {
        return std::nullopt;
    }

    if (!ts.match_punctuation(tokens::Punctuation::Kind::ASSIGNMENT)) {
        ts.unget(*id);
        return std::nullopt;
    }

    auto ty = parse_type(ts);
    if (!ty) {
        return std::nullopt;
    }

    return AliasDecl(std::move(*id), std::move(*ty));
}

std::optional<TypeLit> parse_type_lit(tokens::TokenStream &ts)
{
    if (auto slice = parse_slice_type(ts)) {
        return slice;
    }

    if (auto ptr = parse_pointer_type(ts)) {
        return ptr;
    }

    if (auto map = parse_map_type(ts)) {
        return map;
    }

    if (auto chan = parse_channel_type(ts)) {
        return chan;
    }

    if (auto str = parse_struct_type(ts)) {
        return str;
    }

    return std::nullopt;
}

// Also consumes a trailing semicolon
// FieldDecl is only used in struct types, so this is not an issue
// and is used to resolve some ambiguity
std::optional<StructFieldDecl> parse_struct_field_decl(tokens::TokenStream &ts)
{
    // Need to parse ( IdentifierList Type | EmbeddedField )
    // EmbeddedField and IdentifierList can both begin with an identifier

    // Must be an EmbeddedField
    if (ts.match_punctuation(tokens::Punctuation::Kind::STAR)) {
        auto named_type = parse_named_type(ts);
        if (!named_type) {
            return std::nullopt;
        }

        auto tag = ts.match_consume<tokens::StringLiteral>();
        if (!ts.match_punctuation(tokens::Punctuation::Kind::SEMICOLON)) {
            return std::nullopt;
        }

        StructFieldDecl::EmbeddedField ef{ true, std::move(*named_type) };
        return StructFieldDecl(std::move(ef), tag);
    }

    // Grab as many idents as we can at first
    auto ident_list = parse_identifier_list(ts);

    if (!ident_list) {
        return std::nullopt;
    }

    // If we had more than one identifier, then this must be
    // the first case (with no ambiguity)
    if (ident_list->idents.size() > 1) {
        auto type = parse_type(ts);
        if (!type) {
            return std::nullopt;
        }

        auto tag = ts.match_consume<tokens::StringLiteral>();

        if (!ts.match_punctuation(tokens::Punctuation::Kind::SEMICOLON)) {
            return std::nullopt;
        }

        StructFieldDecl::Field field{ *ident_list, std::make_unique<Type>(
                                                       std::move(*type)) };
        return StructFieldDecl(std::move(field), tag);
    }

    // If we only had one identifier, then the result depends on the next token

    if (auto type = parse_type(ts)) {
        // We found a type, we are in case 1
        auto tag = ts.match_consume<tokens::StringLiteral>();

        if (!ts.match_punctuation(tokens::Punctuation::Kind::SEMICOLON)) {
            return std::nullopt;
        }

        StructFieldDecl::Field field{ *ident_list, std::make_unique<Type>(
                                                       std::move(*type)) };
        return StructFieldDecl(std::move(field), tag);
    }

    // If we didn't find a type, then this must be the type
    // Backtrack and parse it properly

    ts.unget(ident_list->idents.front());

    auto named_type = parse_named_type(ts);
    assert(named_type && "Reparse of named type should always succeed");

    auto tag = ts.match_consume<tokens::StringLiteral>();
    if (!ts.match_punctuation(tokens::Punctuation::Kind::SEMICOLON)) {
        return std::nullopt;
    }

    StructFieldDecl::EmbeddedField ef{ false, std::move(*named_type) };
    return StructFieldDecl(std::move(ef), tag);
}

std::optional<StructType> parse_struct_type(tokens::TokenStream &ts)
{
    if (!ts.match_keyword(tokens::Keyword::Kind::STRUCT)) {
        return std::nullopt;
    }

    if (!ts.match_punctuation(tokens::Punctuation::Kind::LBRACE)) {
        return std::nullopt;
    }

    std::vector<StructFieldDecl> fields;

    while (auto field = parse_struct_field_decl(ts)) {
        fields.push_back(std::move(*field));
    }

    if (!ts.match_punctuation(tokens::Punctuation::Kind::RBRACE)) {
        return std::nullopt;
    }

    return StructType(std::move(fields));
}

std::optional<PointerType> parse_pointer_type(tokens::TokenStream &ts)
{
    if (!ts.match_punctuation(tokens::Punctuation::Kind::STAR)) {
        return std::nullopt;
    }

    auto type = parse_type(ts);
    if (!type) {
        return std::nullopt;
    }

    return PointerType(std::make_unique<Type>(std::move(*type)));
}

std::optional<SliceType> parse_slice_type(tokens::TokenStream &ts)
{
    auto lbracket = ts.match_punctuation(tokens::Punctuation::Kind::LBRACKET);
    if (!lbracket) {
        return std::nullopt;
    }

    if (ts.match_punctuation(tokens::Punctuation::Kind::RBRACKET)) {
        auto ty = parse_type(ts);
        if (ty) {
            return SliceType(std::make_unique<Type>(std::move(*ty)));
        }

        // We matched the rbracket, so it isn't an array
        // Must be an error
        return std::nullopt;
    }

    // Unget the lbracket for possible array parsing
    ts.unget(*lbracket);
    return std::nullopt;
}

std::optional<MapType> parse_map_type(tokens::TokenStream &ts)
{
    if (!ts.match_keyword(tokens::Keyword::Kind::MAP)) {
        return std::nullopt;
    }

    if (!ts.match_punctuation(tokens::Punctuation::Kind::LBRACKET)) {
        return std::nullopt;
    }

    auto key = parse_type(ts);
    if (!key) {
        return std::nullopt;
    }

    if (!ts.match_punctuation(tokens::Punctuation::Kind::RBRACKET)) {
        return std::nullopt;
    }

    auto value = parse_type(ts);
    if (!value) {
        return std::nullopt;
    }

    return MapType(std::make_unique<Type>(std::move(*key)),
                   std::make_unique<Type>(std::move(*value)));
}

std::optional<ChannelType> parse_channel_type(tokens::TokenStream &ts)
{
    if (ts.match_keyword(tokens::Keyword::Kind::CHAN)) {
        auto recv = ts.match_punctuation(tokens::Punctuation::Kind::RECEIVE);
        auto type = parse_type(ts);

        if (!type) {
            return std::nullopt;
        }

        return ChannelType(recv ? ChannelType::Direction::RECV :
                                  ChannelType::Direction::BIDI,
                           std::make_unique<Type>(std::move(*type)));
    }

    if (ts.match_punctuation(tokens::Punctuation::Kind::RECEIVE)) {
        if (!ts.match_keyword(tokens::Keyword::Kind::CHAN)) {
            return std::nullopt;
        }

        if (auto type = parse_type(ts)) {
            return ChannelType(ChannelType::Direction::SEND,
                               std::make_unique<Type>(std::move(*type)));
        }
    }

    return std::nullopt;
}

} // namespace parse

} // namespace goop
