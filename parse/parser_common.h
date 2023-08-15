#ifndef PARSE_PARSER_COMMON_H
#define PARSE_PARSER_COMMON_H

#include "tokens.h"
#include <ostream>

namespace goop {

namespace parse {

struct ASTNode {
    virtual ~ASTNode()
    {
    }
    virtual void print(std::ostream &, int depth) const = 0;
};

template <std::derived_from<ASTNode> T>
std::ostream &operator<<(std::ostream &os, const T &t)
{
    t.print(os, 0);
    return os;
}

struct IdentifierList : public virtual ASTNode {
    std::vector<tokens::Identifier> idents;

    IdentifierList(std::vector<tokens::Identifier> idents) : idents{ idents }
    {
    }

    void print(std::ostream &os, int depth) const override;
};

std::optional<IdentifierList> parse_identifier_list(tokens::TokenStream &ts);

} // namespace parse

} // namespace goop

#endif
