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

typedef struct YYSTYPE {
    char* filename;
    int lineno;
    char* str_lit; // needs to be freed by parser!
    int str_len;
    union {
        unsigned long long integer;
        long double real;
    } number;
    int aux_type;
    int is_signed;
} YYSTYPE;

extern YYSTYPE yylval;

#endif
