#ifndef PARSE_PARSER_H
#define PARSE_PARSER_H

#include "common.h"
#include "type.h"
#include "expr.h"
#include "tokens.h"
#include <cassert>
#include <concepts>
#include <optional>
#include <ostream>
#include <vector>

namespace goop {

namespace parse {

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

} // namespace parse

} // namespace goop

#endif
