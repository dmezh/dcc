#ifndef LEXER_H
#define LEXER_H

#include "tokens-manual.h"

#define HAK_MODE 0

void uint_parse();
int char_parse(int i); // i is position in yytext[] to parse

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
    char* str; // needs to be freed by parser!
    int len;
};

struct context {
    char* filename;
    int lineno;
};

typedef struct YYSTYPE {
    struct number number;
    struct textlit textlit;
} YYSTYPE;

extern YYSTYPE yylval;
extern struct context context;

#endif
