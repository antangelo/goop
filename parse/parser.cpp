#include "parser.h"
#include "parser_common.h"
#include "parser_expr.h"
#include "tokens.h"
#include <optional>

namespace goop {

namespace parse {

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

    return std::nullopt;
}

std::optional<SourceFile> parse_source_file(tokens::TokenStream &ts)
{
    auto pkg = parse_package_clause(ts);
    if (!pkg) {
        return std::nullopt;
    }

    if (!ts.match_punctuation(tokens::Punctuation::Kind::SEMICOLON)) {
        return std::nullopt;
    }

    std::vector<ImportDecl> imports;
    while (auto imp = parse_import_decl(ts)) {
        if (!ts.match_punctuation(tokens::Punctuation::Kind::SEMICOLON)) {
            return std::nullopt;
        }

        imports.push_back(*imp);
    }

    std::vector<TopLevelDecl> decls;
    while (auto decl = parse_top_level_decl(ts)) {
        if (!ts.match_punctuation(tokens::Punctuation::Kind::SEMICOLON)) {
            return std::nullopt;
        }

        decls.push_back(std::move(*decl));
    }

    return SourceFile(*pkg, imports, std::move(decls));
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

            if (!ts.match_punctuation(tokens::Punctuation::Kind::SEMICOLON)) {
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

std::optional<TopLevelDecl> parse_top_level_decl(tokens::TokenStream &ts)
{
    if (auto type_decl = parse_type_decl(ts)) {
        auto tld = std::make_unique<TypeDecl>(std::move(*type_decl));
        return TopLevelDecl(std::move(tld));
    }

    if (auto const_decl = parse_const_decl(ts)) {
        auto tld = std::make_unique<ConstDecl>(std::move(*const_decl));
        return TopLevelDecl(std::move(tld));
    }

    if (auto var_decl = parse_var_decl(ts)) {
        auto tld = std::make_unique<VarDecl>(std::move(*var_decl));
        return TopLevelDecl(std::move(tld));
    }

    return std::nullopt;
}

std::optional<ConstSpec> parse_const_spec(tokens::TokenStream &ts)
{
    auto id = parse_identifier_list(ts);
    if (!id) {
        return std::nullopt;
    }

    std::optional<Type> ty{std::nullopt};
    if (!ts.match_punctuation(tokens::Punctuation::Kind::ASSIGNMENT)) {
        if (ts.peek_punctuation(tokens::Punctuation::Kind::SEMICOLON, tokens::Punctuation::Kind::RPAREN)) {
            return ConstSpec(std::move(*id));
        }

        ty = parse_type(ts);
        if (!ty || !ts.match_punctuation(tokens::Punctuation::Kind::ASSIGNMENT)) {
            return std::nullopt;
        }
    }

    auto expr_list = parse_expression_list(ts);
    return ConstSpec(std::move(*id), std::move(ty), std::move(expr_list));
}

std::optional<ConstDecl> parse_const_decl(tokens::TokenStream &ts)
{
    if (!ts.match_keyword(tokens::Keyword::Kind::CONST)) {
        return std::nullopt;
    }

    std::vector<ConstSpec> specs;

    if (ts.match_punctuation(tokens::Punctuation::Kind::LPAREN)) {
        while (auto spec = parse_const_spec(ts)) {
            specs.push_back(std::move(*spec));

            if (ts.peek_punctuation(tokens::Punctuation::Kind::RPAREN)) {
                break;
            }

            if (!ts.match_punctuation(tokens::Punctuation::Kind::SEMICOLON)) {
                break;
            }
        }

        if (!ts.match_punctuation(tokens::Punctuation::Kind::RPAREN)) {
            return std::nullopt;
        }
    } else {
        auto spec = parse_const_spec(ts);
        if (!spec) {
            return std::nullopt;
        }

        specs.push_back(std::move(*spec));
    }

    return ConstDecl(std::move(specs));
}

std::optional<VarSpec> parse_var_spec(tokens::TokenStream &ts)
{
    auto id = parse_identifier_list(ts);
    if (!id) {
        return std::nullopt;
    }

    std::optional<Type> ty{std::nullopt};
    if (!ts.match_punctuation(tokens::Punctuation::Kind::ASSIGNMENT)) {
        ty = parse_type(ts);
        if (!ty) {
            return std::nullopt;
        }

        if (!ts.match_punctuation(tokens::Punctuation::Kind::ASSIGNMENT)) {
            return VarSpec(std::move(*id), std::move(ty), std::nullopt);
        }
    }

    auto expr_list = parse_expression_list(ts);
    if (!ty && expr_list.is_empty()) {
        return std::nullopt;
    }

    return VarSpec(std::move(*id), std::move(ty), std::move(expr_list));
}

std::optional<VarDecl> parse_var_decl(tokens::TokenStream &ts)
{
    if (!ts.match_keyword(tokens::Keyword::Kind::VAR)) {
        return std::nullopt;
    }

    std::vector<VarSpec> specs;

    if (ts.match_punctuation(tokens::Punctuation::Kind::LPAREN)) {
        while (auto spec = parse_var_spec(ts)) {
            specs.push_back(std::move(*spec));

            if (ts.peek_punctuation(tokens::Punctuation::Kind::RPAREN)) {
                break;
            }

            if (!ts.match_punctuation(tokens::Punctuation::Kind::SEMICOLON)) {
                break;
            }
        }

        if (!ts.match_punctuation(tokens::Punctuation::Kind::RPAREN)) {
            return std::nullopt;
        }
    } else {
        auto spec = parse_var_spec(ts);
        if (!spec) {
            return std::nullopt;
        }

        specs.push_back(std::move(*spec));
    }

    return VarDecl(std::move(specs));
}

} // namespace parse

} // namespace goop
