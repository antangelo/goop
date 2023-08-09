#include "tokens.h"
#include <boost/multiprecision/cpp_int.hpp>
#include <cstdint>
#include <ios>
#include <optional>
#include <set>
#include <string>
#include <unicode/uchar.h>
#include <unicode/urename.h>
#include <unicode/ustream.h>
#include <unicode/unistr.h>
#include <unicode/unum.h>
#include <unicode/schriter.h>
#include <variant>

namespace goop
{

namespace tokens
{

std::optional<int32_t> do_match(UFILE *file, int32_t ch)
{
    u_fungetc(ch, file);
    return std::nullopt;
}

template<typename T, typename ...Args>
std::optional<int32_t> do_match(UFILE *file, int32_t ch, T t, Args... args)
{
    if (ch == t) {
        return ch;
    }

    return do_match(file, ch, args...);
}

template<typename T, typename... Args>
std::optional<int32_t> matches(UFILE *file, T t, Args... args)
{
    auto ch = u_fgetc(file);
    return do_match(file, ch, t, args...);
}

static const std::map<icu::UnicodeString, Keyword::Kind> keyword_map = {
    {"break",       Keyword::Kind::BREAK},
    {"case",        Keyword::Kind::CASE},
    {"chan",        Keyword::Kind::CHAN},
    {"const",       Keyword::Kind::CONST},
    {"continue",    Keyword::Kind::CONTINUE},
    {"default",     Keyword::Kind::DEFAULT},
    {"defer",       Keyword::Kind::DEFER},
    {"else",        Keyword::Kind::ELSE},
    {"fallthrough", Keyword::Kind::FALLTHROUGH},
    {"for",         Keyword::Kind::FOR},
    {"func",        Keyword::Kind::FUNC},
    {"go",          Keyword::Kind::GO},
    {"goto",        Keyword::Kind::GOTO},
    {"if",          Keyword::Kind::IF},
    {"import",      Keyword::Kind::IMPORT},
    {"interface",   Keyword::Kind::INTERFACE},
    {"map",         Keyword::Kind::MAP},
    {"package",     Keyword::Kind::PACKAGE},
    {"range",       Keyword::Kind::RANGE},
    {"return",      Keyword::Kind::RETURN},
    {"select",      Keyword::Kind::SELECT},
    {"struct",      Keyword::Kind::STRUCT},
    {"switch",      Keyword::Kind::SWITCH},
    {"type",        Keyword::Kind::TYPE},
    {"var",         Keyword::Kind::VAR},
};

struct PunctuationParseNode {
    std::optional<Punctuation::Kind> stop;
    std::map<UChar, PunctuationParseNode> node;

    PunctuationParseNode(std::optional<Punctuation::Kind> stop):
        stop{stop}, node{{}} {}
    PunctuationParseNode(std::map<UChar, PunctuationParseNode> node):
        stop{std::nullopt}, node{node} {}
    PunctuationParseNode(std::optional<Punctuation::Kind> stop,
            std::map<UChar, PunctuationParseNode> node):
        stop{stop}, node{node} {}
};

static const std::map<UChar, PunctuationParseNode> punctuation_map = {
    {U'+', PunctuationParseNode(Punctuation::Kind::PLUS, {
            {U'=', PunctuationParseNode(Punctuation::Kind::PLUS_EQUAL)},
            {U'+', PunctuationParseNode(Punctuation::Kind::INCREMENT)},
            })},
    {U'-', PunctuationParseNode(Punctuation::Kind::MINUS, {
            {U'=', PunctuationParseNode(Punctuation::Kind::MINUS_EQUAL)},
            {U'-', PunctuationParseNode(Punctuation::Kind::DECREMENT)},
            })},
    {U'*', PunctuationParseNode(Punctuation::Kind::STAR, {
            {U'=', PunctuationParseNode(Punctuation::Kind::STAR_EQUAL)},
            })},
    {U'/', PunctuationParseNode(Punctuation::Kind::SLASH, {
            {U'=', PunctuationParseNode(Punctuation::Kind::SLASH_EQUAL)},
            })},
    {U'%', PunctuationParseNode(Punctuation::Kind::PERCENT, {
            {U'=', PunctuationParseNode(Punctuation::Kind::MOD_EQUAL)},
            })},
    {U'&', PunctuationParseNode(Punctuation::Kind::AMP, {
            {U'=', PunctuationParseNode(Punctuation::Kind::AND_EQUAL)},
            {U'&', PunctuationParseNode(Punctuation::Kind::BOOL_AND)},
            {U'^', PunctuationParseNode(Punctuation::Kind::BITCLEAR, {
                    {U'=', PunctuationParseNode(Punctuation::Kind::BITCLEAR_EQUAL)},
                    })},
            })},
    {U'|', PunctuationParseNode(Punctuation::Kind::PIPE, {
            {U'=', PunctuationParseNode(Punctuation::Kind::OR_EQUAL)},
            {U'|', PunctuationParseNode(Punctuation::Kind::BOOL_OR)},
            })},
    {U'^', PunctuationParseNode(Punctuation::Kind::CARAT, {
            {U'=', PunctuationParseNode(Punctuation::Kind::XOR_EQUAL)},
            })},
    {U'<', PunctuationParseNode(Punctuation::Kind::LESS_THAN, {
            {U'=', PunctuationParseNode(Punctuation::Kind::LESS_THAN_EQUAL)},
            {U'-', PunctuationParseNode(Punctuation::Kind::RECEIVE)},
            {U'<', PunctuationParseNode(Punctuation::Kind::LSHIFT, {
                    {U'=', PunctuationParseNode(Punctuation::Kind::LSHIFT_EQUAL)},
                    })},
            })},
    {U'>', PunctuationParseNode(Punctuation::Kind::GREATER_THAN, {
            {U'=', PunctuationParseNode(Punctuation::Kind::GREATER_THAN_EQUAL)},
            {U'>', PunctuationParseNode(Punctuation::Kind::RSHIFT, {
                    {U'=', PunctuationParseNode(Punctuation::Kind::RSHIFT_EQUAL)},
                    })},
            })},
    {U'=', PunctuationParseNode(Punctuation::Kind::ASSIGNMENT, {
            {U'=', PunctuationParseNode(Punctuation::Kind::EQUAL)},
            })},
    {U'!', PunctuationParseNode(Punctuation::Kind::BANG, {
            {U'=', PunctuationParseNode(Punctuation::Kind::NOT_EQUAL)},
            })},
    {U':', PunctuationParseNode(Punctuation::Kind::COLON, {
            {U'=', PunctuationParseNode(Punctuation::Kind::SHORT_DECLARATION)},
            })},
    {U'~', PunctuationParseNode(Punctuation::Kind::TILDE)},
    {U'(', PunctuationParseNode(Punctuation::Kind::LPAREN)},
    {U')', PunctuationParseNode(Punctuation::Kind::RPAREN)},
    {U'[', PunctuationParseNode(Punctuation::Kind::LBRACKET)},
    {U']', PunctuationParseNode(Punctuation::Kind::RBRACKET)},
    {U'{', PunctuationParseNode(Punctuation::Kind::LBRACE)},
    {U'}', PunctuationParseNode(Punctuation::Kind::RBRACE)},
    {U';', PunctuationParseNode(Punctuation::Kind::SEMICOLON)},
    {U',', PunctuationParseNode(Punctuation::Kind::COMMA)},
    {U'.', PunctuationParseNode(Punctuation::Kind::DOT, {
            {U'.', PunctuationParseNode({
                    {U'.', PunctuationParseNode(Punctuation::Kind::ELIPSES)},
                    })},
            })},
};

inline bool is_letter(uint16_t c) {
    return u_isalpha(c) || (c == U'_');
}

std::optional<TokenVariant> consume_punctuation(UFILE *file)
{
    const auto *mapping = &punctuation_map;
    std::optional<Punctuation::Kind> candidate;
    UChar ch;

    while ((ch = u_fgetc(file)) != U_EOF) {
            const auto &node = mapping->find(ch);
            if (node != mapping->end()) {
                candidate = node->second.stop;
                mapping = &node->second.node;
            } else {
                break;
            }
    }

    if (ch != U_EOF)
        u_fungetc(ch, file);

    if (candidate) {
        return Punctuation(*candidate);
    }

    return std::nullopt;
}

std::optional<TokenVariant> consume_identifier(UFILE *file)
{
    icu::UnicodeString ident;
    
    auto c = u_fgetc(file);
    if (c == U_EOF || !is_letter(c)) {
        if (c != U_EOF)
            u_fungetc(c, file);
        return std::nullopt;
    }

    do {
        ident.append(c);

        c = u_fgetc(file);
    } while (c != U_EOF && (is_letter(c) || u_isdigit(c)));

    if (c != U_EOF)
        u_fungetc(c, file);

    auto kind_if_keyword = keyword_map.find(ident);
    if (kind_if_keyword != keyword_map.end()) {
        return Keyword(kind_if_keyword->second);
    }

    return Identifier(ident);
}

// Consumes digits from the file into the digits string
// Returns the number of digits consumed and a bool indicating whether all digits were valid in radix
// The actual radix read into the string is max(radix, 10)
std::pair<uint32_t, bool> consume_digits(
        UFILE *file,
        icu::UnicodeString &digits,
        uint8_t radix,
        bool allow_starting_underscore,
        bool last_was_underscore=false
)
{
    UChar next = u_fgetc(file);
    if (next == U_EOF) {
        u_fungetc(next, file);
        return {0, true};
    }

    auto effective_radix = std::max(radix, static_cast<uint8_t>(10));
    bool all_digits_in_radix = true;

    int32_t digit = u_digit(next, effective_radix);
    bool underscore_ok = allow_starting_underscore && next == U'_';
    if (digit == -1 && !underscore_ok) {
        u_fungetc(next, file);
        return {0, all_digits_in_radix};
    }

    all_digits_in_radix &= (underscore_ok || u_digit(next, radix) != -1);
    last_was_underscore = next == U'_';
    if (!last_was_underscore) {
        digits.append(next);
    }

    uint32_t digits_consumed = 1;
    while ((next = u_fgetc(file)) != U_EOF) {
        digit = u_digit(next, effective_radix);
        if (next != U'_' && digit == -1)
            break;

        if (next == U'_' && last_was_underscore)
            break;
        if (last_was_underscore) {
            digits_consumed += 1;
            digits.append(static_cast<UChar>(U'_'));
        }

        last_was_underscore = next == U'_';
        all_digits_in_radix &= (next == U'_' || u_digit(next, radix) != -1);

        if (next != U'_') {
            digits_consumed += 1;
            digits.append(next);
        }
    }

    if (next != U_EOF)
        u_fungetc(next, file);
    
    if (last_was_underscore)
        u_fungetc(U'_', file);

    return {digits_consumed, all_digits_in_radix};
}

std::optional<FloatLiteral> consume_float_literal_with_exponent(
        UFILE *file,
        icu::UnicodeString &digits,
        uint8_t radix,
        std::optional<UChar> has_exponent
)
{
    FloatLiteral literal(
            digits,
            icu::UnicodeString("", "utf-8"),
            radix
            );

    if (has_exponent.has_value()) {
        literal.exponent_char = has_exponent;
        auto optional_sign = matches(file, U'+', U'-');
        literal.negative = optional_sign.has_value() && (*optional_sign == U'-');

        // Exponent radix is always 10
        auto [exponent_digits, all_in_radix] = consume_digits(file, literal.exponent, 10, false);
        // FIXME: Do something about it?
        if (exponent_digits == 0 || !all_in_radix)
            return std::nullopt;
    }

    return literal;
}

std::optional<FloatLiteral> consume_float_literal_after_decimal(
        UFILE *file,
        icu::UnicodeString &digits,
        uint8_t radix,
        bool allow_empty
)
{
    auto digits_consumed = consume_digits(file, digits, radix, false);
    if (false) {
        if (allow_empty) {
            return FloatLiteral(digits, icu::UnicodeString("", "utf-8"), radix);
        }

        u_fungetc(U'.', file);
        return std::nullopt;
    }

    std::optional<UChar> has_exponent;
    assert(radix == 10 || radix == 16);
    if (radix == 10) {
        has_exponent = matches(file, U'e', U'E');
    } else {
        has_exponent = matches(file, U'p', U'P');
    }

    return consume_float_literal_with_exponent(file, digits, radix, has_exponent);
}

std::optional<TokenVariant> consume_numeric_literal(UFILE *file)
{
    enum NumericType {
        Integer,
        Float,
        Imaginary
    };

    icu::UnicodeString digits("", "utf-8"), exponent("", "utf-8");
    uint8_t radix = 10;
    bool radix_implicit = false;
    auto type = NumericType::Integer;

    auto first = u_fgetc(file);
    if (first == U_EOF) {
        u_fungetc(first, file);
        return std::nullopt;
    }

    digits.append(first);
    if (first == U'.') {
        // Must be a decimal float literal
        return consume_float_literal_after_decimal(file, digits, radix, false);
    }

    auto first_digit = u_digit(first, 10);
    if (first_digit < 0)
        return std::nullopt;

    // Detect radix
    auto second = u_fgetc(file);
    bool second_digit_valid = true;
    if (first_digit == 0 && second != U'.') {
        if (auto digit = u_digit(second, 10); digit >= 0) {
            radix = 8;
            radix_implicit = true;

            // Not a valid radix 8 digit,
            // but might be used as a float later
            if (digit > 7)
                second_digit_valid = false;
        } else if (second == U'_') {
            radix = 8;
            radix_implicit = true;
        } else if (second == U'b' || second == U'B') {
            radix = 2;
        } else if (second == U'o' || second == U'O') {
            radix = 8;
        } else if (second == U'x' || second == U'X') {
            radix = 16;
        } else {
            if (second != U_EOF)
                u_fungetc(second, file);
            return IntLiteral(digits, radix);
        }

        digits.append(second);
    } else if (second != U_EOF) {
        u_fungetc(second, file);
    }

    auto [_, all_in_radix] = consume_digits(file, digits, radix, true);
    all_in_radix &= second_digit_valid;

    auto is_float = matches(file, U'.');
    if (is_float) {
        if (radix_implicit) {
            radix = 10;
        }

        if (radix == 8 || radix == 2) {
            return std::nullopt;
        }

        digits.append(*is_float);
        auto literal = consume_float_literal_after_decimal(file, digits, radix, true);

        if (literal && matches(file, U'i')) {
            return ImaginaryLiteral(*literal);
        }

        return literal;
    }

    std::optional<UChar> has_exponent;
    if (radix == 10 || radix_implicit) {
        has_exponent = matches(file, U'e', U'E');
    } else if (radix == 16) {
        has_exponent = matches(file, U'p', U'P');
    }

    if (has_exponent) {
        if (radix_implicit) {
            radix = 10;
        }

        if (radix == 8 || radix == 2) {
            return std::nullopt;
        }

        auto literal = consume_float_literal_with_exponent(file, digits, radix, has_exponent);

        if (literal && matches(file, U'i')) {
            return ImaginaryLiteral(*literal);
        }

        return literal;
    }

    if (!all_in_radix) {
        return std::nullopt;
    }

    IntLiteral literal(
            digits,
            radix
            );

    if (matches(file, U'i')) {
        // Backwards compat clause
        if (radix == 8 && radix_implicit) {
            literal.radix = 10;
        }

        return ImaginaryLiteral(literal);
    }

    return literal;
}

std::optional<RuneLiteral> consume_rune_literal_character(UFILE *file, bool is_string_literal)
{
    static const std::map<UChar, UChar> escaped_values_char = {
        {U'a', U'\a'},
        {U'b', U'\b'},
        {U'f', U'\f'},
        {U'n', U'\n'},
        {U'r', U'\r'},
        {U't', U'\t'},
        {U'v', U'\v'},
        {U'\\', U'\\'},
        {U'\'', U'\''},
    };

    static const std::map<UChar, UChar> escaped_values_string = {
        {U'a', U'\a'},
        {U'b', U'\b'},
        {U'f', U'\f'},
        {U'n', U'\n'},
        {U'r', U'\r'},
        {U't', U'\t'},
        {U'v', U'\v'},
        {U'\\', U'\\'},
        {U'\"', U'\"'},
    };

    const auto &escaped_values = is_string_literal ? escaped_values_string : escaped_values_char;

    if (!matches(file, U'\\')) {
        UChar rune = u_fgetc(file);
        return RuneLiteral(rune, RuneLiteral::Kind::NORMAL);
    }

    if (auto u = matches(file, U'u', U'U')) {
        auto kind = *u == U'U' ? RuneLiteral::Kind::BIG_U : RuneLiteral::Kind::LITTLE_U;

        UChar rune = 0;
        for (int i = 0; i < 4; ++i) {
            auto ch = u_fgetc(file);
            auto digit = u_digit(ch, 16);
            if (digit < 0)
                return std::nullopt;

            rune *= 16;
            rune += digit;
        }

        return RuneLiteral(rune, kind);
    }

    if (matches(file, U'x')) {
        UChar rune = 0;
        for (int i = 0; i < 2; ++i) {
            auto ch = u_fgetc(file);
            auto digit = u_digit(ch, 16);
            if (digit < 0)
                return std::nullopt;

            rune *= 16;
            rune += digit;
        }

        return RuneLiteral(rune, RuneLiteral::Kind::HEX_BYTE);
    }

    UChar next = u_fgetc(file);
    if (next == U_EOF) {
        return std::nullopt;
    }

    auto match = escaped_values.find(next);
    if (match != escaped_values.end()) {
        return RuneLiteral(match->second, RuneLiteral::Kind::ESCAPED_CHAR);
    } else {
        u_fungetc(next, file);
    }

    UChar rune = 0;
    for (int i = 0; i < 3; ++i) {
        auto ch = u_fgetc(file);
        auto digit = u_digit(ch, 8);
        if (digit < 0)
            return std::nullopt;

        rune *= 8;
        rune += digit;
    }

    return RuneLiteral(rune, RuneLiteral::Kind::OCTAL_BYTE);
}

std::optional<RuneLiteral> consume_rune_literal(UFILE *file)
{
    if (!matches(file, U'\'')) {
        return std::nullopt;
    }

    auto rune = consume_rune_literal_character(file, false);

    if (!rune || !matches(file, U'\'')) {
        return std::nullopt;
    }

    return rune;
}

std::optional<TokenVariant> consume_string_literal(UFILE *file)
{
    if (!matches(file, U'"')) {
        return std::nullopt;
    }

    auto string_literal = StringLiteral();

    while (true) {
        if (matches(file, U'"')) {
            return string_literal;
        }

        if (auto rune = consume_rune_literal_character(file, true)) {
            string_literal.runes.push_back(*rune);
        } else {
            break;
        }
    }

    if (!matches(file, U'"')) {
        return std::nullopt;
    }

    return string_literal;
}

std::optional<Comment> consume_comment(UFILE *file)
{
    if (!matches(file, U'/')) {
        return std::nullopt;
    }

    bool multiline = false;
    if (auto ch = matches(file, U'/', U'*')) {
        multiline = *ch == U'*';
    } else {
        u_fungetc(U'/', file);
        return std::nullopt;
    }

    icu::UnicodeString comment("", "utf-8");

    bool multiline_might_end = false;
    while (auto ch = u_fgetc(file)) {
        if (ch == U_EOF)
            break;

        if (!multiline && ch == U'\n')
            break;

        if (multiline_might_end) {
            if (ch == U'/')
                break;

            multiline_might_end = false;
        }

        if (multiline && ch == U'*') {
            multiline_might_end = true;
            continue;
        }

        comment.append(ch);
    }

    return Comment(comment, multiline);
}

TokenStream consume_tokens(UFILE *file)
{
    std::vector<TokenVariant> tokens;

    while (true) {
        auto peek = u_fgetc(file);
        if (peek == U_EOF)
            break;

        if (u_isspace(peek))
            continue;

        u_fungetc(peek, file);

        if (auto token = consume_comment(file)) {
            tokens.push_back(*token);
            continue;
        }

        if (auto token = consume_punctuation(file)) {
            tokens.push_back(*token);
            continue;
        }

        if (auto token = consume_string_literal(file)) {
            tokens.push_back(*token);
            continue;
        }

        if (auto token = consume_rune_literal(file)) {
            tokens.push_back(*token);
            continue;
        }

        if (auto token = consume_identifier(file)) {
            tokens.push_back(*token);
            continue;
        }

        if (auto token = consume_numeric_literal(file)) {
            tokens.push_back(*token);
            continue;
        }
    }

    return TokenStream(tokens);
}

boost::multiprecision::uint256_t IntLiteral::value() const
{
    boost::multiprecision::uint256_t value = 0;
    for (int i = 0; i < lit.length(); ++i) {
        UChar ch = lit.charAt(i);
        auto digit = u_digit(ch, radix);
        if (digit < 0) continue;

        value *= radix;
        value += digit;
    }

    return value;
}

std::ostream &Identifier::operator<<(std::ostream &os) const
{
    os << "Identifier(ident: "
        << this->ident
        << ")";
    return os;
}

std::ostream &IntLiteral::operator<<(std::ostream &os) const
{
    os << "IntLiteral(lit: "
        << this->lit
        << ", value: "
        << this->value()
        << ", radix: "
        << static_cast<unsigned int>(this->radix)
        << ")";
    return os;
}

std::ostream &FloatLiteral::operator<<(std::ostream &os) const
{
    os << "FloatLiteral(mantissa: "
        << this->mantissa
        << ", exponent: "
        << this->exponent
        << ", radix: "
        << static_cast<unsigned int>(this->radix)
        << ", negative_exponent: "
        << (this->negative ? "true" : "false")
        << ")";
    return os;
}

std::ostream &ImaginaryLiteral::operator<<(std::ostream &os) const
{
    os << "ImaginaryLiteral(inner: ";
    std::visit([&](auto &arg) { os << arg; }, this->inner);
    os << ")";
    return os;
}

std::ostream &Keyword::operator<<(std::ostream &os) const
{
    // This doesn't need to be fast, just needs to work
    bool found_keyword = false;
    for (auto &keyword : keyword_map) {
        if (this->kind == keyword.second) {
            os << "Keyword(kind: " << keyword.first << ")";
            found_keyword = true;
        }
    }

    assert(found_keyword && "Invalid keyword");
    return os;
}

std::ostream &Punctuation::operator<<(std::ostream &os) const
{
    static const std::string punctuation_map[] = {
        "+", "-", "*", "/", "%", "&", "|", "^",
        "<<", ">>", "&^", "+=", "-=", "*=", "/=",
        "%=", "&=", "|=", "^=", "<<=", ">>=",
        "&^=", "&&", "||", "<-", "++", "--",
        "==", "<", ">", "=", "!", "~", "!=",
        "<=", ">=", ":=", "...", "(", ")", "[",
        "]", "{", "}", ",", ";", ".", ":",
    };

    os << "Punctuation(kind: ";
    os << punctuation_map[this->kind];
    os << ")";
    return os;
}

std::ostream &RuneLiteral::operator<<(std::ostream &os) const
{
    os << "RuneLiteral(kind: ";

    if (kind == NORMAL) {
        os << "NORMAL, rune: '";
        os << icu::UnicodeString(rune);
    } else if (kind == ESCAPED_CHAR) {
        os << "ESCAPED_CHAR, rune: '\\";
        os << icu::UnicodeString(rune);
    } else if (kind == LITTLE_U || kind == BIG_U) {
        auto u = kind == LITTLE_U ? 'u' : 'U';
        os << "U, rune: '\\";
        os << u;
        os << std::hex << static_cast<uint16_t>(rune) << std::dec;
    } else if (kind == HEX_BYTE) {
        os << "HEX, rune: '\\x";
        os << std::hex << static_cast<uint16_t>(rune) << std::dec;
    } else if (kind == OCTAL_BYTE) {
        os << "OCTAL, rune: '\\";
        os << std::oct << static_cast<uint16_t>(rune) << std::dec;
    }

    os << "')";
    return os;
}

std::ostream &StringLiteral::operator<<(std::ostream &os) const
{
    os << "StringLiteral(literal: \"";

    for (const auto &rune : runes) {
        if (rune.kind == RuneLiteral::Kind::NORMAL) {
            os << icu::UnicodeString(rune.rune);
        } else if (rune.kind == RuneLiteral::Kind::ESCAPED_CHAR) {
            os << icu::UnicodeString(rune.rune);
        } else if (rune.kind == RuneLiteral::Kind::LITTLE_U || rune.kind == RuneLiteral::Kind::BIG_U) {
            auto u = rune.kind == RuneLiteral::Kind::LITTLE_U ? 'u' : 'U';
            os << "\\";
            os << u;
            os << std::hex << static_cast<uint16_t>(rune.rune) << std::dec;
        } else if (rune.kind == RuneLiteral::Kind::HEX_BYTE) {
            os << "\\x";
            os << std::hex << static_cast<uint16_t>(rune.rune) << std::dec;
        } else if (rune.kind == RuneLiteral::Kind::OCTAL_BYTE) {
            os << "\\";
            os << std::oct << static_cast<uint16_t>(rune.rune) << std::dec;
        }
    }

    os << "\")";
    return os;
}

std::ostream &Comment::operator<<(std::ostream &os) const
{
    os << "Comment(multiline: ";
    os << (multiline ? "true" : "false");
    os << ", text: ";
    os << comment;
    os << ")";

    return os;
}

std::ostream &operator<<(std::ostream &os, const TokenVariant &v)
{
    std::visit([&](auto &arg) { os << arg; }, v);
    return os;
}

}

}
