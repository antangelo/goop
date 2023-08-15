#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <unicode/ustream.h>
#include <unicode/urename.h>
#include <unicode/ustdio.h>
#include "tokens.h"
#include "parser.h"

int main() {
    auto *u_stdin = u_finit(stdin, nullptr, nullptr);

    auto tokens = goop::tokens::consume_tokens(u_stdin);
    u_fflush(u_stdin);

    auto source_file = goop::parse::parse_source_file(tokens);
    if (!source_file) {
        std::cerr << "Error parsing source file" << std::endl;
        std::exit(1);
    }

    std::cout << *source_file << std::endl;
}
