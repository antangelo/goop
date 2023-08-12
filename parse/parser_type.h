#ifndef PARSE_PARSER_TYPE_H
#define PARSE_PARSER_TYPE_H

#include <memory>
#include <optional>
#include <cassert>
#include <vector>
#include "util.h"
#include "tokens.h"
#include "parser.h"

namespace goop
{

namespace parse
{

class TypeName : public virtual ASTNode {
    std::optional<tokens::Identifier> package_name;
    tokens::Identifier name;

    public:
    TypeName(tokens::Identifier name):
        package_name{std::nullopt}, name{name} {}

    TypeName(tokens::Identifier package_name, tokens::Identifier name):
        package_name{package_name}, name{name} {}
};

struct Type;

class ArrayType : public virtual ASTNode {};

class StructType : public virtual ASTNode {};

class PointerType : public virtual ASTNode {};

class FunctionType : public virtual ASTNode {};

class InterfaceType : public virtual ASTNode {};

class SliceType : public virtual ASTNode {};

class MapType : public virtual ASTNode {};

struct ChannelType : public virtual ASTNode {
    enum Direction {
        SEND,
        RECV,
        BIDI,
    };

    private:
    Direction direction;
    box<Type> type;

    public:
    ChannelType(Direction direction, std::unique_ptr<Type> &&type):
        direction{direction}, type{std::move(type)} {}
};

typedef std::variant<ArrayType, StructType, PointerType,
        FunctionType, InterfaceType, SliceType,
        MapType, ChannelType> TypeLit;

class TypeList;

struct Type : public virtual ASTNode {
    struct NamedType {
        TypeName name;
        box<TypeList> type_args;

        NamedType(TypeName name): name{name}, type_args{nullptr} {}
        NamedType(TypeName name, box<TypeList> &&tl):
            name{name}, type_args{std::move(tl)} {}
    };

    private:
    std::variant<NamedType, TypeLit> inner;

    public:
    Type(NamedType named): inner{std::move(named)} {}
    Type(TypeLit lit): inner{lit} {}
};

class TypeList : public virtual ASTNode {
    std::vector<Type> types;

    public:
    TypeList(std::vector<Type> types): types{types} {}
};

std::optional<TypeName> parse_type_name(tokens::TokenStream &ts);
std::optional<box<TypeList>> parse_type_list(tokens::TokenStream &ts);
std::optional<box<TypeList>> parse_type_args(tokens::TokenStream &ts);
std::optional<Type> parse_type(tokens::TokenStream &ts);

std::optional<TypeLit> parse_type_lit(tokens::TokenStream &ts);
std::optional<ArrayType> parse_array_type(tokens::TokenStream &ts);
std::optional<StructType> parse_struct_type(tokens::TokenStream &ts);
std::optional<PointerType> parse_pointer_type(tokens::TokenStream &ts);
std::optional<FunctionType> parse_function_type(tokens::TokenStream &ts);
std::optional<InterfaceType> parse_interface_type(tokens::TokenStream &ts);
std::optional<SliceType> parse_slice_type(tokens::TokenStream &ts);
std::optional<MapType> parse_map_type(tokens::TokenStream &ts);
std::optional<ChannelType> parse_channel_type(tokens::TokenStream &ts);

}

}

#endif
