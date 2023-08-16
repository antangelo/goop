#ifndef PARSE_TOKENS_H
#define PARSE_TOKENS_H

#include <boost/multiprecision/cpp_int.hpp>
#include <concepts>
#include <cstdint>
#include <deque>
#include <istream>
#include <map>
#include <optional>
#include <ostream>
#include <unicode/unistr.h>
#include <unicode/ustdio.h>
#include <unicode/ustream.h>
#include <variant>
#include <vector>

namespace goop {

namespace tokens {

enum KeywordKind {
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

enum PunctuationKind {
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

struct Token {
    virtual std::ostream &operator<<(std::ostream &) const = 0;
    virtual bool is_keyword(KeywordKind) const
    {
        return false;
    }

    virtual bool is_punctuation(PunctuationKind) const
    {
        return false;
    }

    virtual bool is_identifier() const
    {
        return false;
    }

    virtual bool is_comment() const
    {
        return false;
    }
};

struct Keyword final : public Token {
    using Kind = KeywordKind;
    Kind kind;

    Keyword(Kind kind) : kind{ kind }
    {
    }
    std::ostream &operator<<(std::ostream &) const override;

    bool is_keyword(KeywordKind kind) const override
    {
        return this->kind == kind;
    }
};

struct Punctuation final : public Token {
    using Kind = PunctuationKind;
    Kind kind;

    Punctuation(Kind kind) : kind{ kind }
    {
    }
    std::ostream &operator<<(std::ostream &) const override;

    bool is_punctuation(PunctuationKind kind) const override
    {
        return this->kind == kind;
    }
};

struct Identifier final : public Token {
    icu::UnicodeString ident;
    Identifier(icu::UnicodeString ident) : ident{ ident }
    {
    }
    std::ostream &operator<<(std::ostream &) const override;

    bool is_identifier() const override
    {
        return true;
    }
};

struct FloatLiteral final : public Token {
    icu::UnicodeString mantissa;
    icu::UnicodeString exponent;
    std::optional<int32_t> exponent_char;
    bool negative;
    uint8_t radix;

    FloatLiteral(icu::UnicodeString mantissa, icu::UnicodeString exponent,
                 uint8_t radix)
        : mantissa{ mantissa }, exponent{ exponent }, negative{ false }, radix{
              radix
          }
    {
    }

    // FIXME
    // boost::multiprecision::uint256_t value() const;
    std::ostream &operator<<(std::ostream &) const override;
};


struct IntLiteral final : public Token {
    icu::UnicodeString lit;
    uint8_t radix;

    IntLiteral(icu::UnicodeString lit, uint8_t radix)
        : lit{ lit }, radix{ radix }
    {
    }

    boost::multiprecision::uint256_t value() const;
    std::ostream &operator<<(std::ostream &) const override;
};

struct ImaginaryLiteral final : public Token {
    std::variant<IntLiteral, FloatLiteral> inner;

    ImaginaryLiteral(std::variant<IntLiteral, FloatLiteral> inner)
        : inner{ inner }
    {
    }

    std::ostream &operator<<(std::ostream &) const override;
};

struct RuneLiteral final : public Token {
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

    RuneLiteral(UChar rune, Kind kind) : rune{ rune }, kind{ kind }
    {
    }

    std::ostream &operator<<(std::ostream &) const override;
};

struct StringLiteral final : public Token {
    std::vector<RuneLiteral> runes;

    StringLiteral()
    {
    }
    std::ostream &operator<<(std::ostream &) const override;
};

struct Comment final : public Token {
    icu::UnicodeString comment;
    bool multiline;

    Comment(icu::UnicodeString comment, bool multiline)
        : comment{ comment }, multiline{ multiline }
    {
    }
    std::ostream &operator<<(std::ostream &) const override;

    bool is_comment() const override
    {
        return true;
    }
};

typedef std::variant<Keyword, Identifier, IntLiteral, FloatLiteral,
                     ImaginaryLiteral, Punctuation, RuneLiteral, StringLiteral,
                     Comment>
    TokenVariant;

static inline Token &token_base(TokenVariant &tok)
{
    return *std::visit([](auto &tv) { return reinterpret_cast<Token *>(&tv); },
                       tok);
}

class TokenStream {
    std::deque<TokenVariant> tokens;

  public:
    TokenStream(std::deque<TokenVariant> tokens) : tokens{ tokens }
    {
    }

    const std::deque<TokenVariant> all() const
    {
        return tokens;
    }

    void skip_comments()
    {
        while (!tokens.empty() && token_base(tokens.front()).is_comment()) {
            tokens.pop_front();
        }
    }

    template <std::derived_from<Token> T> std::optional<T> match_consume()
    {
        skip_comments();
        if (tokens.empty())
            return std::nullopt;

        if (auto *t = std::get_if<T>(&tokens.front())) {
            T tok = *t;
            tokens.pop_front();
            return tok;
        }

        return std::nullopt;
    }

    std::optional<Keyword> match_keyword(Keyword::Kind kind)
    {
        skip_comments();
        if (tokens.empty())
            return std::nullopt;

        if (auto *k = std::get_if<Keyword>(&tokens.front())) {
            if (k->kind == kind) {
                Keyword kw = *k;
                tokens.pop_front();
                return kw;
            }
        }

        return std::nullopt;
    }

    std::optional<Punctuation> match_punctuation(Punctuation::Kind kind)
    {
        skip_comments();
        if (tokens.empty())
            return std::nullopt;

        if (auto *p = std::get_if<Punctuation>(&tokens.front())) {
            if (p->kind == kind) {
                Punctuation punct = *p;
                tokens.pop_front();
                return punct;
            }
        }

        return std::nullopt;
    }

    template<typename K, typename... Args>
    std::optional<Punctuation> match_punctuation(K kind, Args... args)
    {
        if (auto match = match_punctuation(kind)) {
            return match;
        }

        return match_punctuation(args...);
    }

    void unget(TokenVariant t)
    {
        tokens.push_front(t);
    }
};

std::optional<Punctuation> consume_punctuation(UFILE *file);
std::optional<TokenVariant> consume_identifier(UFILE *file);
std::optional<TokenVariant> consume_numeric_literal(UFILE *file);
std::optional<RuneLiteral> consume_rune_literal(UFILE *file);
std::optional<TokenVariant> consume_string_literal(UFILE *file);
std::optional<Comment> consume_comment(UFILE *file);

TokenStream consume_tokens(UFILE *file);

std::ostream &operator<<(std::ostream &os, const TokenVariant &v);


template <std::derived_from<Token> Tok>
std::ostream &operator<<(std::ostream &os, const Tok &t)
{
    t.operator<<(os);
    return os;
}

} // namespace tokens

} // namespace goop

#endif
