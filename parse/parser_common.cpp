#include "parser_common.h"
#include "tokens.h"
#include <optional>
#include <vector>

namespace goop
{

namespace parse
{

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

}

}
