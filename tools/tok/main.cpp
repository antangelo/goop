#include <cstdio>
#include <iostream>
#include <unicode/ustream.h>
#include <unicode/unistr.h>
#include <unicode/ustdio.h>
#include "tokens.h"

void parse_old(UFILE *u_stdin)
{
    int32_t c;
    while ((c = u_fgetc(u_stdin)) != U_EOF) {
        if (c == U'\n')
            continue;

        if (false && c == U'/') {
            auto maybe_comment = u_fgetc(u_stdin);
            if (maybe_comment == U'/') {
                while ((c = u_fgetc(u_stdin)) != U_EOF) {
                    if (c == U'\n') break;
                }

                continue;
            } else {
                u_fungetc(maybe_comment, u_stdin);
            }
        }

        u_fungetc(c, u_stdin);

        auto comment = goop::tokens::consume_comment(u_stdin);
        if (comment) {
            std::cout << *comment << std::endl;
            continue;
        }

        auto punct = goop::tokens::consume_punctuation(u_stdin);
        if (punct.has_value()) {
            std::cout << *punct << std::endl;
            continue;
        }

        auto strl = goop::tokens::consume_string_literal(u_stdin);
        if (strl.has_value()) {
            std::cout << *strl << std::endl;
            continue;
        }

        auto rune = goop::tokens::consume_rune_literal(u_stdin);
        if (rune.has_value()) {
            std::cout << *rune << std::endl;
            continue;
        }

        auto ident = goop::tokens::consume_identifier(u_stdin);
        if (ident.has_value()) {
            std::cout << *ident << std::endl;
            continue;
        }

        auto int_lit = goop::tokens::consume_numeric_literal(u_stdin);
        if (int_lit.has_value()) {
            std::cout << *int_lit << std::endl;
            continue;
        }
    }
}

int main() {
    auto *u_stdin = u_finit(stdin, nullptr, nullptr);

#if 0
    parse_old(u_stdin);
    u_fflush(u_stdin);
    return 0;
#endif

    auto tokens = goop::tokens::consume_tokens(u_stdin);
    for (const auto &token : tokens.all()) {
        std::cout << token << std::endl;
    }

    u_fflush(u_stdin);
}
