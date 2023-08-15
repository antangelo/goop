#ifndef PARSE_PARSER_TYPE_H
#define PARSE_PARSER_TYPE_H

#include "parser.h"
#include "parser_common.h"
#include "tokens.h"
#include <cassert>
#include <memory>
#include <optional>
#include <vector>

namespace goop {

namespace parse {

class TypeName : public virtual ASTNode {
    std::optional<tokens::Identifier> package_name;
    tokens::Identifier name;

  public:
    TypeName(tokens::Identifier name)
        : package_name{ std::nullopt }, name{ name }
    {
    }

    TypeName(tokens::Identifier package_name, tokens::Identifier name)
        : package_name{ package_name }, name{ name }
    {
    }

    void print(std::ostream &os, int depth) const override;
};

class Type;
class TypeList;

struct NamedType {
    TypeName name;
    std::unique_ptr<TypeList> type_args;

    NamedType(TypeName name) : name{ name }, type_args{ nullptr }
    {
    }
    NamedType(TypeName name, std::unique_ptr<TypeList> &&tl)
        : name{ name }, type_args{ std::move(tl) }
    {
    }

    void print(std::ostream &os, int depth) const;
};

class ArrayType : public virtual ASTNode {
    void print(std::ostream &os, int depth) const override
    {
        assert(false);
    }
};

struct StructFieldDecl {
    struct EmbeddedField {
        bool pointer;
        NamedType type;
    };

    struct Field {
        IdentifierList idents;
        std::unique_ptr<Type> type;
    };

  private:
    std::variant<EmbeddedField, Field> inner;
    std::optional<tokens::StringLiteral> tag;

  public:
    StructFieldDecl(std::variant<EmbeddedField, Field> inner,
                    std::optional<tokens::StringLiteral> tag)
        : inner{ std::move(inner) }, tag{ tag }
    {
    }

    void print(std::ostream &os, int depth) const;
};

class StructType : public virtual ASTNode {
    std::vector<StructFieldDecl> fields;

  public:
    StructType(std::vector<StructFieldDecl> fields)
        : fields{ std::move(fields) }
    {
    }

    void print(std::ostream &os, int depth) const override;
};

class PointerType : public virtual ASTNode {
    std::unique_ptr<Type> inner;

  public:
    PointerType(std::unique_ptr<Type> inner) : inner{ std::move(inner) }
    {
    }

    void print(std::ostream &os, int depth) const override;
};

class FunctionType : public virtual ASTNode {
    void print(std::ostream &os, int depth) const override
    {
        assert(false);
    }
};

class InterfaceType : public virtual ASTNode {
    void print(std::ostream &os, int depth) const override
    {
        assert(false);
    }
};

class SliceType : public virtual ASTNode {
    std::unique_ptr<Type> inner;

  public:
    SliceType(std::unique_ptr<Type> inner) : inner{ std::move(inner) }
    {
    }

    void print(std::ostream &os, int depth) const override;
};

class MapType : public virtual ASTNode {
    std::unique_ptr<Type> key, value;

  public:
    MapType(std::unique_ptr<Type> key, std::unique_ptr<Type> value)
        : key{ std::move(key) }, value{ std::move(value) }
    {
    }

    void print(std::ostream &os, int depth) const override;
};

struct ChannelType : public virtual ASTNode {
    enum Direction {
        SEND,
        RECV,
        BIDI,
    };

  private:
    Direction direction;
    std::unique_ptr<Type> type;

  public:
    ChannelType(Direction direction, std::unique_ptr<Type> &&type)
        : direction{ direction }, type{ std::move(type) }
    {
    }

    void print(std::ostream &os, int depth) const override;
};

typedef std::variant<ArrayType, StructType, PointerType, FunctionType,
                     InterfaceType, SliceType, MapType, ChannelType>
    TypeLit;

class Type : public virtual ASTNode {
    std::variant<NamedType, TypeLit> inner;

  public:
    Type(NamedType named) : inner{ std::move(named) }
    {
    }
    Type(TypeLit lit) : inner{ std::move(lit) }
    {
    }

    void print(std::ostream &os, int depth) const override;
};

class TypeList : public virtual ASTNode {
    std::vector<Type> types;

  public:
    TypeList(std::vector<Type> types) : types{ std::move(types) }
    {
    }

    void print(std::ostream &os, int depth) const override;
};

struct TypeSpec : public virtual ASTNode {
    virtual bool is_alias() const
    {
        return false;
    }
};

class TypeDef : public virtual TypeSpec {
  public:
    void print(std::ostream &os, int depth) const override;
};

class AliasDecl : public virtual TypeSpec {
    tokens::Identifier id;
    Type ty;

  public:
    AliasDecl(tokens::Identifier id, Type ty) : id{ id }, ty{ std::move(ty) }
    {
    }

    bool is_alias() const override
    {
        return true;
    }

    void print(std::ostream &os, int depth) const override;
};

class TypeDecl : public virtual ASTNode {
    std::vector<std::unique_ptr<TypeSpec>> types;

  public:
    TypeDecl(std::vector<std::unique_ptr<TypeSpec>> types)
        : types{ std::move(types) }
    {
    }
    void print(std::ostream &os, int depth) const override;
};

std::optional<TypeName> parse_type_name(tokens::TokenStream &ts);
std::optional<std::unique_ptr<TypeList>>
parse_type_list(tokens::TokenStream &ts);
std::optional<std::unique_ptr<TypeList>>
parse_type_args(tokens::TokenStream &ts);
std::optional<Type> parse_type(tokens::TokenStream &ts);

std::optional<TypeDecl> parse_type_decl(tokens::TokenStream &ts);
std::optional<TypeDef> parse_type_def(tokens::TokenStream &ts);
std::optional<AliasDecl> parse_alias_decl(tokens::TokenStream &ts);

std::optional<TypeLit> parse_type_lit(tokens::TokenStream &ts);
std::optional<ArrayType> parse_array_type(tokens::TokenStream &ts);
std::optional<StructType> parse_struct_type(tokens::TokenStream &ts);
std::optional<PointerType> parse_pointer_type(tokens::TokenStream &ts);
std::optional<FunctionType> parse_function_type(tokens::TokenStream &ts);
std::optional<InterfaceType> parse_interface_type(tokens::TokenStream &ts);
std::optional<SliceType> parse_slice_type(tokens::TokenStream &ts);
std::optional<MapType> parse_map_type(tokens::TokenStream &ts);
std::optional<ChannelType> parse_channel_type(tokens::TokenStream &ts);

} // namespace parse

} // namespace goop

#endif
