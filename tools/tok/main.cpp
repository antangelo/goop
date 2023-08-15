#include "tokens.h"
#include <cstdio>
#include <iostream>
#include <unicode/unistr.h>
#include <unicode/ustdio.h>
#include <unicode/ustream.h>

int main()
{
    auto *u_stdin = u_finit(stdin, nullptr, nullptr);

    auto tokens = goop::tokens::consume_tokens(u_stdin);
    for (const auto &token : tokens.all()) {
        std::cout << token << std::endl;
    }

    u_fflush(u_stdin);
}
