#include <stdlib.h>
#include <stdio.h>
#include "ast.h"

astn* astn_alloc(enum astn_type type) {
    astn *n = (astn*)malloc(sizeof(astn));
    n->type = type;
    return n;
}

void print_ast(astn *n) {
    static int tabs = 0;
    for (int i=0; i<tabs; i++) printf("  ");

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
            printf("BINARY OP %c\n", n->astn_binop.op);
            tabs++;
            print_ast(n->astn_binop.left);
            print_ast(n->astn_binop.right);
            tabs--; return;
        case ASTN_DEREF:
            printf("DEREF\n");
            tabs++;
            print_ast(n->astn_deref.target);
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
    }
}
