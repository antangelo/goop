#ifndef PARSE_PARSER_STATEMENT_H
#define PARSE_PARSER_STATEMENT_H

#include "common.h"
#include "parser/expr.h"
#include "parser/decl.h"
#include "tokens.h"
#include <memory>
#include <vector>
#include <optional>

namespace goop {

namespace parse {

struct Statement : public virtual ASTNode {
    virtual ~Statement() {};

    virtual bool is_terminating() {
        return false;
    }
};

struct LabeledStatement : public virtual Statement {
    tokens::Identifier label;
    std::unique_ptr<Statement> statement;

    bool is_terminating() override {
        return statement->is_terminating();
    }

    LabeledStatement(tokens::Identifier label,
            std::unique_ptr<Statement> statement):
        label{std::move(label)}, statement{std::move(statement)} {}
};

class DeclarationStatement : public virtual Statement {
    std::variant<ConstDecl, VarDecl> decl;

    public:
    DeclarationStatement(std::variant<ConstDecl, VarDecl> decl):
        decl{std::move(decl)} {}
};

class GoStatement : public virtual Statement {
    std::unique_ptr<Expression> expr;

    public:
    GoStatement(std::unique_ptr<Expression> expr):
        expr{std::move(expr)} {}
};

class DeferStatement : public virtual Statement {
    std::unique_ptr<Expression> expr;

    public:
    DeferStatement(std::unique_ptr<Expression> expr):
        expr{std::move(expr)} {}
};

class ReturnStatement : public virtual Statement {
    ExpressionList values;

    public:
    ReturnStatement(ExpressionList values):
        values{std::move(values)} {}
};

class GotoStatement : public virtual Statement {
    tokens::Identifier label;

    public:
    GotoStatement(tokens::Identifier label):
        label{std::move(label)} {}
};

class ContinueStatement : public virtual Statement {
    std::optional<tokens::Identifier> label;

    public:
    ContinueStatement(std::optional<tokens::Identifier> label):
        label{std::move(label)} {}
};

class BreakStatement : public virtual Statement {
    std::optional<tokens::Identifier> label;

    public:
    BreakStatement(std::optional<tokens::Identifier> label):
        label{std::move(label)} {}
};

class FallthroughStatement : public virtual Statement {};

struct SimpleStatement : public virtual Statement {
    virtual ~SimpleStatement() {};
};

class EmptyStatement : public virtual SimpleStatement {};

class SendStatement : public virtual SimpleStatement {
    using Expr = std::unique_ptr<Expression>;
    Expr channel, expr;

    public:
    SendStatement(Expr channel, Expr expr):
        channel{std::move(channel)}, expr{std::move(expr)} {}
};

class IncrDecrStatement : public virtual SimpleStatement {
    std::unique_ptr<Expression> expr;
    bool increment;

    public:
    IncrDecrStatement(std::unique_ptr<Expression> expr, bool increment):
        expr{std::move(expr)}, increment{increment} {}
};

class AssignmentStatement : public virtual SimpleStatement {
    ExpressionList lhs, rhs;
    tokens::Punctuation op;

    public:
    AssignmentStatement(ExpressionList lhs, ExpressionList rhs,
            tokens::Punctuation op):
        lhs{std::move(lhs)}, rhs{std::move(rhs)}, op{op} {}
};

class ExpressionStatement : public virtual SimpleStatement {
    std::unique_ptr<Expression> expr;

    public:
    ExpressionStatement(std::unique_ptr<Expression> expr):
        expr{std::move(expr)} {}
};

struct StatementList : public virtual ASTNode {
    std::vector<std::unique_ptr<Statement>> list;

    bool is_terminating() {
        return !list.empty() &&
            list.back()->is_terminating();
    }
};

struct Block : public virtual Statement {
    StatementList list;

    bool is_terminating() override {
        return list.is_terminating();
    }
};

std::unique_ptr<SimpleStatement> parse_simple_statement(tokens::TokenStream &ts);
std::unique_ptr<Statement> parse_statement(tokens::TokenStream &ts);
std::unique_ptr<Block> parse_block(tokens::TokenStream &ts);

}

}

#endif
