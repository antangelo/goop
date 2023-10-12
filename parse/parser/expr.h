#ifndef PARSE_PARSER_EXPR_H
#define PARSE_PARSER_EXPR_H

#include "common.h"
#include "type.h"
#include "tokens.h"
#include <cstdint>
#include <memory>
#include <variant>
#include <vector>

namespace goop {

namespace parse {

class Type;

struct Expression : public virtual ASTNode {
    virtual ~Expression() {}

    virtual void print(std::ostream &, int) const override = 0;
};

class ExpressionList : public virtual ASTNode {
    std::vector<std::unique_ptr<Expression>> exps;

    public:
    ExpressionList(std::vector<std::unique_ptr<Expression>> exps):
        exps{std::move(exps)} {}

    void print(std::ostream &, int) const override;

    bool is_empty() const;
    size_t size() const;

    std::unique_ptr<Expression> &operator[](size_t);
};

struct BasicLiteral : public virtual ASTNode {
    using Literal = std::variant<tokens::IntLiteral,
          tokens::FloatLiteral, tokens::ImaginaryLiteral,
          tokens::RuneLiteral, tokens::StringLiteral>;

    private:
    Literal lit;

    public:
    BasicLiteral(Literal lit): lit{lit} {}

    void print(std::ostream &, int) const override;
};

class NamedOperand : public virtual ASTNode {
    IdentOrQualified name;
    std::unique_ptr<TypeList> type_args;

    public:
    NamedOperand(IdentOrQualified name, std::unique_ptr<TypeList> type_args):
        name{std::move(name)}, type_args{std::move(type_args)} {}

    void print(std::ostream &, int) const override;
};

class Selector : public virtual ASTNode {
    tokens::Identifier ident;

    public:
    Selector(tokens::Identifier ident):
        ident{std::move(ident)} {}

    void print(std::ostream &, int) const override;
};

class Index : public virtual ASTNode {
    std::unique_ptr<Expression> inner;

    public:
    Index(std::unique_ptr<Expression> inner):
        inner{std::move(inner)} {}

    void print(std::ostream &, int) const override;
};

class Slice : public virtual ASTNode {
    std::unique_ptr<Expression> low, high, max;

    public:
    Slice(): low{nullptr}, high{nullptr}, max{nullptr} {}
    Slice(std::unique_ptr<Expression> low, std::unique_ptr<Expression> high):
        low{std::move(low)}, high{std::move(high)}, max{nullptr} {}
    Slice(std::unique_ptr<Expression> low, std::unique_ptr<Expression> high,
        std::unique_ptr<Expression> max):
        low{std::move(low)}, high{std::move(high)}, max{std::move(max)} {}

    void print(std::ostream &, int) const override;
};

class TypeAssertion : public virtual ASTNode {
    std::unique_ptr<Type> type;

    public:
    TypeAssertion(std::unique_ptr<Type> type):
        type{std::move(type)} {}

    void print(std::ostream &, int) const override;
};

class Arguments : public virtual ASTNode {
    ExpressionList exps;
    bool elipses;

    public:
    Arguments(ExpressionList exps, bool elipses):
        exps{std::move(exps)}, elipses{elipses} {}

    void print(std::ostream &, int) const override;
};

// PrimaryExpression -> (Expression)
class ParenExpression : public virtual ASTNode {
    std::unique_ptr<Expression> inner;

    public:
    ParenExpression(std::unique_ptr<Expression> inner):
        inner{std::move(inner)} {}

    void print(std::ostream &, int) const override;
};

struct PrimaryExpression : public virtual ASTNode {
    /*
     * Here be dragons!
     * There is ambiguity in Golang's grammar here. Namely:
     *
     * An IdentOrQualified can be any of:
     *  Operand (via OperandName)
     *  Conversion (via ReceiverType instanceof NamedType)
     *  MethodExpr (with ReceiverType instanceof TypeName unqualified)
     *
     * An expression of the below form is also ambiguous:
     *   Operand Arguments -> (Expression) (Expression)
     *   Conversion -> (Type) (Expression)
     *
     * To rectify this we make the following alterations to the grammar
     * and resolve them during context enrichment:
     *   1. Parse IdentOrQualified separately to isolate it
     *   2. Allow TypeLiteral to be an expression variant.
     *   Note that this makes Type a subset of Expression
     */

    using Inner = std::variant<IdentOrQualified, TypeLit, ParenExpression,
          BasicLiteral, NamedOperand>;
    using Outer = std::variant<Selector, Index, Slice, TypeAssertion, Arguments>;

    private:
    Inner inner;
    std::vector<Outer> outers;

    public:
    PrimaryExpression(Inner inner, std::vector<Outer> outers):
        inner{std::move(inner)}, outers{std::move(outers)} {}

    void print(std::ostream &, int) const override;
};


class UnaryExpression : public virtual Expression {
    PrimaryExpression expr;
    std::vector<tokens::Punctuation> unary_ops;

    public:
    UnaryExpression(PrimaryExpression expr, std::vector<tokens::Punctuation> unary_ops):
        expr{std::move(expr)}, unary_ops{std::move(unary_ops)} {}
    UnaryExpression(PrimaryExpression expr, tokens::Punctuation unary_op):
        expr{std::move(expr)}, unary_ops{std::move(unary_op)} {}

    void print(std::ostream &, int) const override;
};

class BinaryExpression : public virtual Expression {
    tokens::Punctuation op;
    std::unique_ptr<Expression> lhs, rhs;

    public:
    BinaryExpression(tokens::Punctuation op, std::unique_ptr<Expression> lhs,
        std::unique_ptr<Expression> rhs):
        op{op}, lhs{std::move(lhs)}, rhs{std::move(rhs)} {}

    void print(std::ostream &, int) const override;
};

std::unique_ptr<Expression> parse_expression(tokens::TokenStream &ts);
ExpressionList parse_expression_list(tokens::TokenStream &ts);
std::optional<UnaryExpression> parse_unary_expression(tokens::TokenStream &ts);

std::optional<PrimaryExpression> parse_primary_expression(tokens::TokenStream &ts);

} // namespace parse

} // namespace goop

#endif
