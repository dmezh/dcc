#include <stdio.h>
#include "lex.yy.c"
#include "tokens-string.h"
#include "lexer.h"

void printtok(int token) {
    printf("%s\t%d\t", context.filename ? context.filename : "<stdin>", context.lineno);
    if (token < 257) {
        putchar(token);
        putchar('\n');
        return;
    }
    switch(token)
    {
            case NUMBER:
                printf("NUMBER\t");
                if (yylval.number.aux_type > s_REAL)
                    printf("REAL\t%f\t", yylval.number.real);
                else 
                    printf("INTEGER\t%llu\t", yylval.number.integer);
                if (!yylval.number.is_signed) printf("UNSIGNED,");
                printf("%s\n", int_types_str[yylval.number.aux_type]);
                return;
            case STRING:
                printf("STRING\t%s\n", yylval.textlit.str);
                return;
            case CHARLIT:
                printf("CHARLIT\t%c\n", yylval.textlit.str[0]);
                return;
            case IDENT:
                printf("IDENT\t%s\n", yylval.ident);
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
