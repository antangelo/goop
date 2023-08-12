#include "parser.h"
#include "tokens.h"

namespace goop
{

namespace parse
{

std::optional<PackageClause> parse_package_clause(tokens::TokenStream &ts)
{
    auto kw = ts.match_keyword(tokens::Keyword::Kind::PACKAGE);
    if (!kw) {
        return std::nullopt;
    }

    auto package_name = ts.match_consume<tokens::Identifier>();
    if (package_name && package_name->ident.length() > 0) {
        return PackageClause(*package_name);
    }

    ts.unget(*kw);
    return std::nullopt;
}

std::optional<SourceFile> parse_source_file(tokens::TokenStream &ts)
{
    return std::nullopt;
}

std::optional<ImportSpec> parse_import_spec(tokens::TokenStream &ts)
{
    bool dot = false;
    std::optional<tokens::Identifier> package_name = std::nullopt;

    if (ts.match_punctuation(tokens::Punctuation::Kind::DOT)) {
        dot = true;
    } else if (auto package = ts.match_consume<tokens::Identifier>()) {
        package_name = package;
    }

    auto import_path = ts.match_consume<tokens::StringLiteral>();
    if (!import_path) {
        return std::nullopt;
    }

    return ImportSpec(*import_path, package_name, dot);
}

std::optional<ImportDecl> parse_import_decl(tokens::TokenStream &ts)
{
    auto kw_import = ts.match_keyword(tokens::Keyword::Kind::IMPORT);
    if (!kw_import) {
        return std::nullopt;
    }

    if (ts.match_punctuation(tokens::Punctuation::Kind::LPAREN)) {
        std::vector<ImportSpec> import_specs;

        while (auto spec = parse_import_spec(ts)) {
            import_specs.push_back(*spec);

            if (!ts.match_punctuation(tokens::Punctuation::SEMICOLON)) {
                break;
            }
        }

        if (ts.match_punctuation(tokens::Punctuation::Kind::RPAREN)) {
            return ImportDecl(import_specs);
        }
    } else {
        auto import_spec = parse_import_spec(ts);
        if (import_spec) {
            return ImportDecl(*import_spec);
        }
    }

    ts.unget(*kw_import);
    return std::nullopt;
}

}

}
