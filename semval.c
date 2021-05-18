#include "semval.h"

#include "charutil.h"

#include <stdio.h>

const char* int_types_str[] = {
    "UNSPEC",
    "CHARLIT",
    "INT",
    "LONG",
    "LONGLONG",
    "REAL", // just used for logic, shouldn't be assigned
    "FLOAT",
    "DOUBLE",
    "LONGDOUBLE",
};

void print_number(const struct number *n, FILE* w) {
    if (n->aux_type == s_CHARLIT) {
        printf("CHARLIT: '"); emit_char(n->integer, w); printf("'\n");
    } else {
        printf("CONSTANT (");
        if (!n->is_signed) printf("UNSIGNED ");
        printf("%s): ", int_types_str[n->aux_type]);
        if (n->aux_type < s_REAL)
            printf("%llu", n->integer);
        else
            printf("%Lg\n", n->real);
    }
}

void print_number_e(const struct number *n) {
    if (n->aux_type == s_CHARLIT) {
        fprintf(stderr, "CHARLIT: '"); emit_char(n->integer, stderr); fprintf(stderr, "'\n");
    } else {
        fprintf(stderr, "CONSTANT (");
        if (!n->is_signed) fprintf(stderr, "UNSIGNED ");
        fprintf(stderr, "%s): ", int_types_str[n->aux_type]);
        if (n->aux_type < s_REAL)
            fprintf(stderr, "%llu", n->integer);
        else
            fprintf(stderr, "%Lg\n", n->real);
    }
}
