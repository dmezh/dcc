#include <stdio.h>
#include "lex.yy.c"

int main() {
    int t;
    while (t=yylex())
    {
        switch(t)
        {
            case NUMBER:
                printf("NUMBER: %d\n", yylval);
                break;
            case PLUS:
                printf("PLUS\n");
                break;
            case MINUS:
                printf("MINUS\n");
                break;
        }
    }
    printf("EOF\n");
}