#include <stdio.h>
#include "lex.yy.c"
#include "tokens-string.h"
#include "lexer.h"

void printtok(int token) {
    if (token < 257) {
        putchar(token);
        putchar('\n');
        return;
    }
    printf("(filename)\t%d\t", yylval.lineno);
    switch(token)
    {
            case NUMBER:
                printf("NUMBER\t");
                if (yylval.aux_type > s_REAL)
                    printf("REAL\t%f\t", yylval.number.real);
                else 
                    printf("INTEGER\t%llu\t", yylval.number.integer);
                if (!yylval.is_signed) printf("UNSIGNED,");
                printf("%s\n", int_types_str[yylval.aux_type]);
                return;
            case STRING:
                printf("STRING\t%s\n", yylval.str_lit);
                return;
            case CHARLIT:
                printf("CHARLIT\t%c\n", yylval.str_lit[0]);
                return;
            default:
                printf("%s\n", token ? tokens_str[token - 256] : tokens_str[0]);
    }
    return;
}

int main() {
    int t;
    while (t=yylex())
        printtok(t);
    printf("EOF\n");
}
