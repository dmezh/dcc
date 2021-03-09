#ifndef LEXER_H
#define LEXER_H

#include "../semval.h"
#include "../2-parser/ast.h"
#include "../2-parser/parser.tab.h"

#define HAK_MODE 0
#define FRIENDLYFN context.filename ? context.filename : "<stdin>"

int yylex();
int process_uint();
int process_oct();
int process_real();
unsigned char parse_char_safe(char* str, int* i);
int char_parse(int i); // i is position in yytext[] to parse
void print_context(int warn);

struct context {
    char* filename;
    int lineno;
};

extern struct context context;

#endif
