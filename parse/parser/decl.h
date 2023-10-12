#ifndef PARSE_PARSER_DECL_H
#define PARSE_PARSER_DECL_H

#include "common.h"
#include "type.h"
#include "expr.h"
#include <optional>

namespace goop {

namespace parse {

class ConstSpec : public virtual ASTNode {
    IdentifierList idents;
    std::optional<Type> type;
    std::optional<ExpressionList> exprs;

    public:
    ConstSpec(IdentifierList idents):
        idents{std::move(idents)}, type{std::nullopt},
        exprs{std::nullopt} {}
    ConstSpec(IdentifierList idents, std::optional<Type> type):
        idents{std::move(idents)}, type{std::move(type)},
        exprs{std::nullopt} {}
    ConstSpec(IdentifierList idents, std::optional<Type> type, std::optional<ExpressionList> exprs):
        idents{std::move(idents)}, type{std::move(type)},
        exprs{std::move(exprs)} {}
    ConstSpec(IdentifierList idents, std::optional<ExpressionList> exprs):
        idents{std::move(idents)}, type{std::nullopt},
        exprs{std::move(exprs)} {}

    void print(std::ostream &os, int depth) const override;
};

class ConstDecl : public virtual ASTNode {
    std::vector<ConstSpec> decls;

    public:
    ConstDecl(std::vector<ConstSpec> decls): decls{std::move(decls)} {}
    void print(std::ostream &os, int depth) const override;
};

class VarSpec : public virtual ASTNode {
    IdentifierList idents;
    std::optional<Type> type;
    std::optional<ExpressionList> exprs;

    public:
    VarSpec(IdentifierList idents):
        idents{std::move(idents)}, type{std::nullopt},
        exprs{std::nullopt} {}
    VarSpec(IdentifierList idents, std::optional<Type> type):
        idents{std::move(idents)}, type{std::move(type)},
        exprs{std::nullopt} {}
    VarSpec(IdentifierList idents, std::optional<Type> type, std::optional<ExpressionList> exprs):
        idents{std::move(idents)}, type{std::move(type)},
        exprs{std::move(exprs)} {}
    VarSpec(IdentifierList idents, std::optional<ExpressionList> exprs):
        idents{std::move(idents)}, type{std::nullopt},
        exprs{std::move(exprs)} {}

    void print(std::ostream &os, int depth) const override;
};

class VarDecl : public virtual ASTNode {
    std::vector<VarSpec> decls;

    public:
    VarDecl(std::vector<VarSpec> decls): decls{std::move(decls)} {}
    void print(std::ostream &os, int depth) const override;
};


std::optional<ConstSpec> parse_const_spec(tokens::TokenStream &ts);
std::optional<ConstDecl> parse_const_decl(tokens::TokenStream &ts);
std::optional<VarSpec> parse_var_spec(tokens::TokenStream &ts);
std::optional<VarDecl> parse_var_decl(tokens::TokenStream &ts);

}

}

#endif
