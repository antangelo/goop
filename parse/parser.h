#ifndef PARSE_PARSER_H
#define PARSE_PARSER_H

#include <optional>
#include <cassert>
#include <vector>
#include "tokens.h"

namespace goop
{

namespace parse
{

class ASTNode {};

class PackageClause : public virtual ASTNode {
    tokens::Identifier package_name;

    public:
    PackageClause(tokens::Identifier package_name):
        package_name{package_name} {}
};

class ImportSpec {
    tokens::StringLiteral path;
    std::optional<tokens::Identifier> package_name;
    bool dot;

    public:
    ImportSpec(tokens::StringLiteral path):
        path{path}, package_name{std::nullopt}, dot{false} {}

    ImportSpec(tokens::StringLiteral path, std::optional<tokens::Identifier> package_name, bool dot):
        path{path}, package_name{package_name}, dot{dot} {
        assert(!(package_name && dot));
    }
};

class ImportDecl : public virtual ASTNode {
    std::vector<ImportSpec> import_specs;

    public:
    ImportDecl(std::vector<ImportSpec> import_specs):
        import_specs{import_specs} {}
    ImportDecl(ImportSpec import_spec):
        import_specs{import_spec} {}
};

class TopLevelDecl : public virtual ASTNode {
};

class SourceFile : public virtual ASTNode {
    PackageClause package;
    std::vector<ImportDecl> imports;
    std::vector<TopLevelDecl> top_level_decls;
};

std::optional<SourceFile> parse_source_file(tokens::TokenStream &ts);
std::optional<PackageClause> parse_package_clause(tokens::TokenStream &ts);

std::optional<ImportDecl> parse_import_decl(tokens::TokenStream &ts);
std::optional<ImportSpec> parse_import_spec(tokens::TokenStream &ts);

std::optional<TopLevelDecl> parse_top_level_decl(tokens::TokenStream &ts);

}

}

#endif
