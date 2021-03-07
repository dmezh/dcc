#ifndef LEXER_H
#define LEXER_H

#include "tokens-manual.h"

#define HAK_MODE 0
#define FRIENDLYFN context.filename ? context.filename : "<stdin>"

int process_uint();
int process_oct();
void real_parse();
int char_parse(int i); // i is position in yytext[] to parse
void print_context(int warn);

enum int_types {
    s_UNSPEC,
    s_INT,
    s_LONG,
    s_LONGLONG,
    s_REAL, // just used for logic, shouldn't be assigned
    s_FLOAT,
    s_DOUBLE,
    s_LONGDOUBLE,
};

/*
typedef union {
    char* str_lit;
    int integer;
} YYSTYPE;
*/

struct number {
    unsigned long long integer;
    long double real;
    int aux_type;
    int is_signed;
};

struct textlit {
    char* str;
    int len;
};

struct context {
    char* filename;
    int lineno;
};

typedef union YYSTYPE {
    struct number number;
    struct textlit textlit;
    char* ident;
} YYSTYPE;

extern YYSTYPE yylval;
extern struct context context;

#endif
