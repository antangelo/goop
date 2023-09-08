#include "parser.h"
#include "parser_common.h"
#include "parser_type.h"
#include "parser_expr.h"
#include "tokens.h"
#include <cassert>
#include <ostream>

namespace goop {

namespace parse {

template <class... Ts> struct overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

static inline void indent(std::ostream &os, int depth)
{
    for (int i = 0; i < depth; ++i) {
        os << "  ";
    }
}

void PackageClause::print(std::ostream &os, int depth) const
{
    indent(os, depth);
    os << "PackageClause [ package = ";
    os << package_name;
    os << "]";
    os << std::endl;
};

void ImportSpec::print(std::ostream &os, int depth) const
{
    indent(os, depth);
    os << "ImportSpec [ path = ";
    os << path;
    os << ", package_name = ";
    if (package_name) {
        os << *package_name;
    } else if (dot) {
        os << ".";
    } else {
        os << "None";
    }

    os << " ]" << std::endl;
}

void ImportDecl::print(std::ostream &os, int depth) const
{
    indent(os, depth);
    os << "ImportDecl [" << std::endl;

    for (const auto &is : import_specs) {
        is.print(os, depth + 1);
    }

    indent(os, depth);
    os << "]" << std::endl;
}

void TopLevelDecl::print(std::ostream &os, int depth) const
{
    indent(os, depth);

    os << "TopLevelDecl [" << std::endl;
    node->print(os, depth + 1);

    indent(os, depth);
    os << "]" << std::endl;
}

void SourceFile::print(std::ostream &os, int depth) const
{
    indent(os, depth);

    os << "SourceFile [" << std::endl;

    package.print(os, depth + 1);

    for (const auto &id : imports) {
        id.print(os, depth + 1);
    }

    for (const auto &tld : top_level_decls) {
        tld.print(os, depth + 1);
    }

    indent(os, depth);
    os << "]" << std::endl;
}

void IdentifierList::print(std::ostream &os, int depth) const
{
    indent(os, depth);
    os << "IdentifierList [" << std::endl;

    for (const auto &ident : idents) {
        indent(os, depth + 1);
        os << ident << std::endl;
    }

    indent(os, depth);
    os << "]" << std::endl;
}

void IdentOrQualified::print(std::ostream &os, int depth) const
{
    indent(os, depth);
    os << "IdentOrQualified [ package = ";
    if (package_name) {
        os << *package_name;
    } else {
        os << "None";
    }
    os << ", name = ";
    os << name;
    os << "]" << std::endl;
}

void NamedType::print(std::ostream &os, int depth) const
{
    indent(os, depth);
    os << "NamedType [" << std::endl;

    name.print(os, depth + 1);

    if (type_args) {
        type_args->print(os, depth + 1);
    }

    indent(os, depth);
    os << "]" << std::endl;
}

void Type::print(std::ostream &os, int depth) const
{
    indent(os, depth);
    os << "Type [" << std::endl;

    std::visit(overloaded{
                   [&](const TypeLit &a) {
                       reinterpret_cast<const ASTNode *>(&a)->print(os,
                                                                    depth + 1);
                   },
                   [&](const NamedType &a) { a.print(os, depth + 1); },
               },
               inner);

    indent(os, depth);
    os << "]" << std::endl;
}

void TypeDef::print(std::ostream &os, int depth) const
{
    indent(os, depth);
    os << "TypeDef [" << std::endl;

    indent(os, depth);
    os << "]" << std::endl;
}

void AliasDecl::print(std::ostream &os, int depth) const
{
    indent(os, depth);
    os << "AliasDecl [ ident = " << id;
    os << ", ty =" << std::endl;

    ty.print(os, depth + 1);

    indent(os, depth);
    os << "]" << std::endl;
}

void TypeDecl::print(std::ostream &os, int depth) const
{
    indent(os, depth);
    os << "TypeDecl [" << std::endl;

    for (const auto &t : types) {
        t->print(os, depth + 1);
    }

    indent(os, depth);
    os << "]" << std::endl;
}

void StructFieldDecl::print(std::ostream &os, int depth) const
{
    indent(os, depth);
    os << "StructFieldDecl [" << std::endl;

    indent(os, depth + 1);
    std::visit(overloaded{
                   [&](const EmbeddedField &ef) {
                       os << "EmbeddedField [ ptr: ";
                       os << (ef.pointer ? "true" : "false") << std::endl;

                       ef.type.print(os, depth + 2);
                   },
                   [&](const Field &f) {
                       os << "Field [ idents = " << std::endl;
                       f.idents.print(os, depth + 2);

                       indent(os, depth + 1);
                       os << "ty = " << std::endl;
                       f.type->print(os, depth + 2);
                   },
               },
               inner);

    indent(os, depth + 1);
    os << "]";

    indent(os, depth);
    os << "]" << std::endl;
}

void StructType::print(std::ostream &os, int depth) const
{
    indent(os, depth);
    os << "StructType [" << std::endl;

    for (const auto &f : fields) {
        f.print(os, depth + 1);
    }

    indent(os, depth);
    os << "]" << std::endl;
}

void PointerType::print(std::ostream &os, int depth) const
{
    indent(os, depth);
    os << "PointerType [ inner: " << std::endl;

    inner->print(os, depth + 1);

    indent(os, depth);
    os << "]" << std::endl;
}

void SliceType::print(std::ostream &os, int depth) const
{
    indent(os, depth);
    os << "SliceType [ inner: " << std::endl;

    inner->print(os, depth + 1);

    indent(os, depth);
    os << "]" << std::endl;
}

void MapType::print(std::ostream &os, int depth) const
{
    indent(os, depth);
    os << "MapType [ key: " << std::endl;

    key->print(os, depth + 1);

    indent(os, depth);
    os << ", value: " << std::endl;

    value->print(os, depth + 1);

    indent(os, depth);
    os << "]" << std::endl;
}

void ChannelType::print(std::ostream &os, int depth) const
{
    indent(os, depth);
    os << "ChannelType [ direction: ";

    switch (direction) {
    case Direction::SEND:
        os << "SEND";
        break;
    case Direction::RECV:
        os << "RECV";
        break;
    case Direction::BIDI:
        os << "BIDI";
        break;
    }

    os << ", inner: " << std::endl;

    type->print(os, depth + 1);

    indent(os, depth);
    os << "]" << std::endl;
}

void TypeList::print(std::ostream &os, int depth) const
{
    indent(os, depth);
    os << "TypeList [" << std::endl;

    for (const auto &t : types) {
        t.print(os, depth + 1);
    }

    indent(os, depth);
    os << "]" << std::endl;
}

void ExpressionList::print(std::ostream &os, int depth) const
{
    indent(os, depth);
    os << "ExpressionList [" << std::endl;

    for (const auto &e : exps) {
        e->print(os, depth + 1);
    }

    indent(os, depth);
    os << "]" << std::endl;
}

void BasicLiteral::print(std::ostream &os, int depth) const
{
    indent(os, depth);
    os << "BasicLiteral [";

    std::visit(overloaded {
            [&](const auto &tok) {
            reinterpret_cast<const tokens::Token *>(&tok)->operator<<(os);
            }
            }, lit);

    os << "]" << std::endl;
}

void NamedOperand::print(std::ostream &os, int depth) const
{
    indent(os, depth);
    os << "NamedOperand [ name = " << std::endl;
    
    name.print(os, depth + 1);

    indent(os, depth);
    os << "type = ";
    if (type_args) {
        os << std::endl;
        type_args->print(os, depth + 1);
    } else {
        os << "None" << std::endl;
    }

    indent(os, depth);
    os << "]" << std::endl;
}

void Selector::print(std::ostream &os, int depth) const
{
    indent(os, depth);
    os << "Selector [ ident = " << ident << "]" << std::endl;
}

void Index::print(std::ostream &os, int depth) const
{
    indent(os, depth);
    os << "Index [" << std::endl;

    assert(inner);
    inner->print(os, depth + 1);

    indent(os, depth);
    os << "]" << std::endl;
}

void Slice::print(std::ostream &os, int depth) const
{
    indent(os, depth);
    os << "Slice [" << std::endl;

    if (low) {
        low->print(os, depth + 1);
    }

    if (high) {
        high->print(os, depth + 1);
    }

    if (max) {
        max->print(os, depth + 1);
    }

    indent(os, depth);
    os << "]" << std::endl;
}

void TypeAssertion::print(std::ostream &os, int depth) const
{
    indent(os, depth);
    os << "TypeAssertion [" << std::endl;

    assert(type);
    type->print(os, depth + 1);

    indent(os, depth);
    os << "]" << std::endl;
}

void Arguments::print(std::ostream &os, int depth) const
{
    indent(os, depth);
    os << "Arguments [ elipses = ";
    os << (elipses ? "true" : "false") << std::endl;

    exps.print(os, depth + 1);

    indent(os, depth);
    os << "]" << std::endl;
}

void ParenExpression::print(std::ostream &os, int depth) const
{
    indent(os, depth);
    os << "ParenExpression [" << std::endl;

    assert(inner);
    inner->print(os, depth + 1);

    indent(os, depth);
    os << "]" << std::endl;
}

void PrimaryExpression::print(std::ostream &os, int depth) const
{
    indent(os, depth);
    os << "PrimaryExpression [" << std::endl;

    std::visit(overloaded {
            [&](const TypeLit &tl) {
                reinterpret_cast<const ASTNode *>(&tl)->print(os, depth + 1);
            },
            [&](const auto &ast) {
                ast.print(os, depth + 1);
            }
            }, inner);

    for (const auto &outer : outers) {
        std::visit(overloaded {
                [&](const auto &ast) {
                ast.print(os, depth + 1);
                }
                }, outer);
    }

    indent(os, depth);
    os << "]" << std::endl;
}

void UnaryExpression::print(std::ostream &os, int depth) const
{
    indent(os, depth);
    os << "UnaryExpression [ ops = ";

    for (const auto &op : unary_ops) {
        os << op << ", ";
    }

    os << "expr = " << std::endl;

    expr.print(os, depth + 1);

    indent(os, depth);
    os << "]" << std::endl;
}

void BinaryExpression::print(std::ostream &os, int depth) const
{
    indent(os, depth);
    os << "BinaryExpression [ op = " << op << ", lhs = " << std::endl;

    assert(lhs && rhs);
    lhs->print(os, depth + 1);

    indent(os, depth);
    os << "rhs = " << std::endl;

    rhs->print(os, depth + 1);

    indent(os, depth);
    os << "]" << std::endl;
}

void ConstSpec::print(std::ostream &os, int depth) const
{
    indent(os, depth);
    os << "ConstSpec [ idents = " << std::endl;

    idents.print(os, depth + 1);

    if (type) {
        indent(os, depth);
        os << "type = " << std::endl;
        type->print(os, depth + 1);
    }

    if (exprs) {
        indent(os, depth);
        os << "exprs = " << std::endl;
        exprs->print(os, depth + 1);
    }

    indent(os, depth);
    os << "]" << std::endl;
}

void ConstDecl::print(std::ostream &os, int depth) const
{
    indent(os, depth);
    os << "ConstDecl [ " << std::endl;

    for (const auto &spec : decls) {
        spec.print(os, depth + 1);
    }

    indent(os, depth);
    os << "]" << std::endl;
}

void VarSpec::print(std::ostream &os, int depth) const
{
    indent(os, depth);
    os << "VarSpec [ idents = " << std::endl;

    idents.print(os, depth + 1);

    if (type) {
        indent(os, depth);
        os << "type = " << std::endl;
        type->print(os, depth + 1);
    }

    if (exprs) {
        indent(os, depth);
        os << "exprs = " << std::endl;
        exprs->print(os, depth + 1);
    }

    indent(os, depth);
    os << "]" << std::endl;
}

void VarDecl::print(std::ostream &os, int depth) const
{
    indent(os, depth);
    os << "VarDecl [ " << std::endl;

    for (const auto &spec : decls) {
        spec.print(os, depth + 1);
    }

    indent(os, depth);
    os << "]" << std::endl;
}

} // namespace parse

} // namespace goop
