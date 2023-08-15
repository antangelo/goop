#ifndef PARSE_PARSER_EXPR_H
#define PARSE_PARSER_EXPR_H

#include "parser_common.h"
#include <variant>

namespace goop
{

namespace parse
{

class Expression : public virtual ASTNode
{
};

std::optional<std::monostate> parse_expression(tokens::TokenStream &ts);
std::optional<std::monostate> parse_unary_expression(tokens::TokenStream &ts);
std::optional<std::monostate> parse_primary_expression(tokens::TokenStream &ts);

}

}

#endif
