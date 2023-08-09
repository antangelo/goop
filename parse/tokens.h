#ifndef PARSE_TOKENS_H
#define PARSE_TOKENS_H

#include <map>
#include <cstdint>
#include <istream>
#include <optional>
#include <concepts>
#include <ostream>
#include <variant>
#include <unicode/unistr.h>
#include <unicode/ustdio.h>
#include <unicode/ustream.h>
#include <boost/multiprecision/cpp_int.hpp>

namespace goop
{

namespace tokens
{


struct Token {
    virtual std::ostream &operator<<(std::ostream &) const = 0;
};

struct Keyword : public Token {
    enum Kind {
        BREAK,
        CASE,
        CHAN,
        CONST,
        CONTINUE,
        DEFAULT,
        DEFER,
        ELSE,
        FALLTHROUGH,
        FOR,
        FUNC,
        GO,
        GOTO,
        IF,
        IMPORT,
        INTERFACE,
        MAP,
        PACKAGE,
        RANGE,
        RETURN,
        SELECT,
        STRUCT,
        SWITCH,
        TYPE,
        VAR,
    };

    Kind kind;

    Keyword(Kind kind): kind{kind} {}
    std::ostream &operator<<(std::ostream &) const override;
};

struct Punctuation : public Token {
    enum Kind {
        PLUS,
        MINUS,
        STAR,
        SLASH,
        PERCENT,
        AMP,
        PIPE,
        CARAT,
        LSHIFT,
        RSHIFT,
        BITCLEAR,
        PLUS_EQUAL,
        MINUS_EQUAL,
        STAR_EQUAL,
        SLASH_EQUAL,
        MOD_EQUAL,
        AND_EQUAL,
        OR_EQUAL,
        XOR_EQUAL,
        LSHIFT_EQUAL,
        RSHIFT_EQUAL,
        BITCLEAR_EQUAL,
        BOOL_AND,
        BOOL_OR,
        RECEIVE,
        INCREMENT,
        DECREMENT,
        EQUAL,
        LESS_THAN,
        GREATER_THAN,
        ASSIGNMENT,
        BANG,
        TILDE,
        NOT_EQUAL,
        LESS_THAN_EQUAL,
        GREATER_THAN_EQUAL,
        SHORT_DECLARATION,
        ELIPSES,
        LPAREN,
        RPAREN,
        LBRACKET,
        RBRACKET,
        LBRACE,
        RBRACE,
        COMMA,
        SEMICOLON,
        DOT,
        COLON,
    };

    Kind kind;

    Punctuation(Kind kind): kind{kind} {}
    std::ostream &operator<<(std::ostream &) const override;
};

struct Identifier : public Token {
    icu::UnicodeString ident;
    Identifier(icu::UnicodeString ident): ident{ident} {}
    std::ostream &operator<<(std::ostream &) const override;
};

struct FloatLiteral : public Token {
    icu::UnicodeString mantissa;
    icu::UnicodeString exponent;
    std::optional<int32_t> exponent_char;
    bool negative;
    uint8_t radix;

    FloatLiteral(icu::UnicodeString mantissa, icu::UnicodeString exponent, uint8_t radix):
        mantissa{mantissa}, exponent{exponent}, negative{false}, radix{radix} {}

    //FIXME
    //boost::multiprecision::uint256_t value() const;
    std::ostream &operator<<(std::ostream &) const override;
};


struct IntLiteral : public Token {
    icu::UnicodeString lit;
    uint8_t radix;

    IntLiteral(icu::UnicodeString lit, uint8_t radix):
        lit{lit}, radix{radix} {}

    boost::multiprecision::uint256_t value() const;
    std::ostream &operator<<(std::ostream &) const override;
};

struct ImaginaryLiteral : public Token {
    std::variant<IntLiteral, FloatLiteral> inner;

    ImaginaryLiteral(std::variant<IntLiteral, FloatLiteral> inner):
        inner{inner} {}

    std::ostream &operator<<(std::ostream &) const override;
};

struct RuneLiteral : public Token {
    enum Kind {
        NORMAL,
        LITTLE_U,
        BIG_U,
        OCTAL_BYTE,
        HEX_BYTE,
        ESCAPED_CHAR,
    };

    UChar rune;
    Kind kind;

    RuneLiteral(UChar rune, Kind kind): rune{rune}, kind{kind} {}

    std::ostream &operator<<(std::ostream &) const override;
};

struct StringLiteral : public Token {
    std::vector<RuneLiteral> runes;

    StringLiteral() {}
    std::ostream &operator<<(std::ostream &) const override;
};

struct Comment : public Token {
    icu::UnicodeString comment;
    bool multiline;

    Comment(icu::UnicodeString comment, bool multiline):
        comment{comment}, multiline{multiline} {}
    std::ostream &operator<<(std::ostream &) const override;
};

typedef std::variant<Keyword, Identifier, IntLiteral,
        FloatLiteral, ImaginaryLiteral, Punctuation,
        RuneLiteral, StringLiteral, Comment> TokenVariant;

class TokenStream {
    std::vector<TokenVariant> tokens;

    public:
    TokenStream(std::vector<TokenVariant> tokens):
        tokens{tokens} {}

    const std::vector<TokenVariant> all() const {
        return tokens;
    }
};

std::optional<TokenVariant> consume_punctuation(UFILE *file);
std::optional<TokenVariant> consume_identifier(UFILE *file);
std::optional<TokenVariant> consume_numeric_literal(UFILE *file);
std::optional<RuneLiteral> consume_rune_literal(UFILE *file);
std::optional<TokenVariant> consume_string_literal(UFILE *file);
std::optional<Comment> consume_comment(UFILE *file);

TokenStream consume_tokens(UFILE *file);

std::ostream &operator<<(std::ostream &os, const TokenVariant &v);

template<std::derived_from<Token> Tok>
std::ostream &operator<<(std::ostream &os, const Tok &t)
{
    t.operator<<(os);
    return os;
}

}

}

#endif
