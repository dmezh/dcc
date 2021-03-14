#include <stdio.h>
#include "tokens-string.h"
#include "lexer.h"
#include "charutil.h"

void printtok(int token) {
    printf("%s\t%d\t", FRIENDLYFN, context.lineno);
    if (token < 257) {
        putchar(token);
        putchar('\n');
        return;
    }
    switch(token)
    {
            case NUMBER:
                if (yylval.number.aux_type == t_CHARLIT) {
                    printf("CHARLIT\t");
                    emit_char(yylval.number.integer);
                    printf("\n");
                    return;
                } else {
                    printf("NUMBER\t");
                    if (yylval.number.aux_type > t_REAL) {
                        printf("REAL\t%Lg\t", yylval.number.real);
                    } 
                    else {
                        printf("INTEGER\t%llu\t", yylval.number.integer);
                        if (!yylval.number.is_signed) printf("UNSIGNED,");
                    }
                    printf("%s\n", int_types_str[yylval.number.aux_type]);
                    return;
                }
            case STRING:
                printf("STRING\t");
                for (int i = 0; i<yylval.strlit.len; i++) {
                    emit_char(yylval.strlit.str[i]);
                }
                printf("\n");
                return;
            case IDENT:
                printf("IDENT\t%s\n", yylval.ident);
                return;
            default:
                printf("%s\n", token ? tokens_str[token - 255] : tokens_str[0]);
    }
    return;
}

int main() {
    int t;
    while ((t=yylex()))
        printtok(t);
    // printf("EOF\n");
}
