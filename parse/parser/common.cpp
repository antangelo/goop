#include "common.h"
#include "tokens.h"
#include <optional>
#include <vector>

namespace goop {

namespace parse {

std::optional<IdentifierList> parse_identifier_list(tokens::TokenStream &ts)
{
    std::vector<tokens::Identifier> idents;

    do {
        if (auto ident = ts.match_consume<tokens::Identifier>()) {
            idents.push_back(*ident);
        } else {
            return std::nullopt;
        }
    } while (ts.match_punctuation(tokens::Punctuation::Kind::COMMA));

    if (idents.empty())
        return std::nullopt;

    return IdentifierList(idents);
}

std::optional<IdentOrQualified> parse_ident_or_qualified(tokens::TokenStream &ts)
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

        return IdentOrQualified(*name_or_package, *name);
    }

    return IdentOrQualified(*name_or_package);
}

} // namespace parse

} // namespace goop
