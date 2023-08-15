#include "parser.h"
#include "parser_common.h"
#include "parser_type.h"

namespace goop {

namespace parse {

static inline void indent(std::ostream &os, int depth)
{
    for (int i = 0; i < depth; ++i) {
        os << "\t";
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

void TypeName::print(std::ostream &os, int depth) const
{
    indent(os, depth);
    os << "TypeName [ package = ";
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

    std::visit(tokens::TokenStream::overloaded{
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
    std::visit(tokens::TokenStream::overloaded{
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

} // namespace parse

} // namespace goop
