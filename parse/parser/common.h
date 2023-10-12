#ifndef PARSE_PARSER_COMMON_H
#define PARSE_PARSER_COMMON_H

#include "tokens.h"
#include <cassert>
#include <ostream>

namespace goop {

namespace parse {

struct ASTNode {
    virtual ~ASTNode()
    {
    }

    virtual void print(std::ostream &, int) const
    {
        assert(!"ASTNode printed without overriding print");
    }

    virtual bool is_ambiguous() const
    {
        return false;
    }
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

class IdentOrQualified : public virtual ASTNode {
    std::optional<tokens::Identifier> package_name;
    tokens::Identifier name;

  public:
    IdentOrQualified(tokens::Identifier name)
        : package_name{ std::nullopt }, name{ name }
    {
    }

    IdentOrQualified(tokens::Identifier package_name, tokens::Identifier name)
        : package_name{ package_name }, name{ name }
    {
    }

    void print(std::ostream &os, int depth) const override;
};

std::optional<IdentifierList> parse_identifier_list(tokens::TokenStream &ts);
std::optional<IdentOrQualified> parse_ident_or_qualified(tokens::TokenStream &ts);

} // namespace parse

} // namespace goop

#endif
