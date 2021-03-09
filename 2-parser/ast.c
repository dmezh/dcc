#include <stdlib.h>
#include <stdio.h>
#include "ast.h"
#include "parser.tab.h"

astn* astn_alloc(enum astn_type type) {
    astn *n = (astn*)malloc(sizeof(astn));
    n->type = type;
    return n;
}

void print_ast(astn *n) {
    static int tabs = 0;     //     -> __ <- two spaces
    for (int i=0; i<tabs; i++) printf("  ");
    if (!n) return; // if we just want to print tabs (ASTN_TERN)
    switch (n->type) {
        case ASTN_NUM:
            printf("CONSTANT (");
            if (!n->astn_num.number.is_signed) printf("UNSIGNED ");
            printf("%s): ", int_types_str[n->astn_num.number.aux_type]);
            if (n->astn_num.number.aux_type < s_REAL)
                printf("%llu\n", n->astn_num.number.integer);
            else
                printf("%Lg", n->astn_num.number.real);
            return;
        case ASTN_ASSIGN:
            printf("ASSIGNMENT\n");
            tabs++;
                print_ast(n->astn_assign.left);
                print_ast(n->astn_assign.right);
            tabs--; return;
        case ASTN_IDENT:
            printf("IDENT: %s\n", n->astn_ident.ident);
            return;
        case ASTN_STRLIT:
            printf("STRING:\t%s\n", n->astn_strlit.strlit.str);
            return;
        case ASTN_BINOP:
            printf("BINARY OP ");
            switch (n->astn_binop.op) {
                case SHL:   printf("<<\n"); break;
                case SHR:   printf(">>\n"); break;
                case LTEQ:  printf("<=\n"); break;
                case GTEQ:  printf(">=\n"); break;
                case EQEQ:  printf("==\n"); break;
                case NOTEQ: printf("!=\n"); break;
                case LOGAND:printf("&&\n"); break;
                case LOGOR: printf("||\n"); break;
                default:    printf("%c\n", n->astn_binop.op); break;
            }
            tabs++;
                print_ast(n->astn_binop.left);
                print_ast(n->astn_binop.right);
            tabs--; return;
        case ASTN_FNCALL: // wip
            printf("FNCALL\n");
            return;
        case ASTN_SELECT:
            printf("SELECT\n");
            tabs++;
                print_ast(n->astn_select.parent);
                print_ast(n->astn_select.member);
            tabs--; return;
        case ASTN_UNOP:
            printf("UNOP ");
            switch (n->astn_unop.op) {
                case PLUSPLUS:      printf("POSTINC\n");                break;
                case MINUSMINUS:    printf("POSTDEC\n");                break;
                case '*':           printf("DEREF\n");                  break;
                case '&':           printf("ADDRESSOF\n");              break;
                default:            printf("%c\n", n->astn_unop.op);    break;
            }
            tabs++;
                print_ast(n->astn_unop.target);
            tabs--; return;
        case ASTN_SIZEOF:
            printf("SIZEOF\n");
            tabs++;
                print_ast(n->astn_sizeof.target);
            tabs--; return;
        case ASTN_TERN: // 
            printf("TERNARY\n");
            tabs++;
                print_ast(0); printf("IF:\n");
                tabs++; print_ast(n->astn_tern.cond); tabs--;

                print_ast(0); printf("THEN:\n");
                tabs++; print_ast(n->astn_tern.t_then); tabs--;

                print_ast(0); printf("ELSE:\n");
                tabs++; print_ast(n->astn_tern.t_else); tabs--;
            tabs--; return;
    }
}
