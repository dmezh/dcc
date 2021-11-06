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

void print_number(const struct number *n, FILE* f) {
    if (n->aux_type == s_CHARLIT) {
        fprintf(f, "CHARLIT: '"); emit_char(n->integer, f); fprintf(f, "\'");
    } else {
        fprintf(f, "CONSTANT (");
        if (!n->is_signed) fprintf(f, "UNSIGNED ");
        fprintf(f, "%s): ", int_types_str[n->aux_type]);
        if (n->aux_type < s_REAL)
            fprintf(f, "%llu", n->integer);
        else
            fprintf(f, "%Lg", n->real);
    }
}
