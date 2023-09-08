#ifndef PARSE_PARSER_H
#define PARSE_PARSER_H

#include "parser_common.h"
#include "parser_type.h"
#include "parser_expr.h"
#include "tokens.h"
#include <cassert>
#include <concepts>
#include <optional>
#include <ostream>
#include <vector>

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

class PackageClause : public virtual ASTNode {
    tokens::Identifier package_name;

  public:
    PackageClause(tokens::Identifier package_name)
        : package_name{ package_name }
    {
    }

    void print(std::ostream &os, int depth) const override;
};

class ImportSpec : public virtual ASTNode {
    tokens::StringLiteral path;
    std::optional<tokens::Identifier> package_name;
    bool dot;

  public:
    ImportSpec(tokens::StringLiteral path)
        : path{ path }, package_name{ std::nullopt }, dot{ false }
    {
    }

    ImportSpec(tokens::StringLiteral path,
               std::optional<tokens::Identifier> package_name, bool dot)
        : path{ path }, package_name{ package_name }, dot{ dot }
    {
        assert(!(package_name && dot));
    }

    void print(std::ostream &os, int depth) const override;
};

class ImportDecl : public virtual ASTNode {
    std::vector<ImportSpec> import_specs;

  public:
    ImportDecl(std::vector<ImportSpec> import_specs)
        : import_specs{ import_specs }
    {
    }
    ImportDecl(ImportSpec import_spec) : import_specs{ import_spec }
    {
    }

    void print(std::ostream &os, int depth) const override;
};

class TopLevelDecl : public virtual ASTNode {
    std::unique_ptr<ASTNode> node;

  public:
    TopLevelDecl(std::unique_ptr<ASTNode> node) : node{ std::move(node) }
    {
    }

    void print(std::ostream &os, int depth) const override;
};

class SourceFile : public virtual ASTNode {
    PackageClause package;
    std::vector<ImportDecl> imports;
    std::vector<TopLevelDecl> top_level_decls;

  public:
    SourceFile(PackageClause package, std::vector<ImportDecl> imports,
               std::vector<TopLevelDecl> top_level_decls)
        : package{ package }, imports{ imports }, top_level_decls{ std::move(
                                                      top_level_decls) }
    {
    }

    void print(std::ostream &os, int depth) const override;
};


std::optional<SourceFile> parse_source_file(tokens::TokenStream &ts);
std::optional<PackageClause> parse_package_clause(tokens::TokenStream &ts);

std::optional<ImportDecl> parse_import_decl(tokens::TokenStream &ts);
std::optional<ImportSpec> parse_import_spec(tokens::TokenStream &ts);

std::optional<TopLevelDecl> parse_top_level_decl(tokens::TokenStream &ts);

std::optional<ConstSpec> parse_const_spec(tokens::TokenStream &ts);
std::optional<ConstDecl> parse_const_decl(tokens::TokenStream &ts);
std::optional<VarSpec> parse_var_spec(tokens::TokenStream &ts);
std::optional<VarDecl> parse_var_decl(tokens::TokenStream &ts);

} // namespace parse

} // namespace goop

#endif
