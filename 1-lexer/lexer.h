#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>

#define FRIENDLYFN context.filename ? context.filename : "<stdin>"

int yylex();
void print_context(bool warn);

struct context {
    char* filename;
    int lineno;
};

extern struct context context;

#endif
