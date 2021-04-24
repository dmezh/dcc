/*
 * semval.h
 *
 * Core definitions for semantic values.
 */

#ifndef SEMVAL_H
#define SEMVAL_H

#include <stdbool.h>
#include <stddef.h>

enum int_types {
    s_UNSPEC,
    s_CHARLIT,
    s_INT,
    s_LONG,
    s_LONGLONG,
    s_REAL, // just used for logic, shouldn't be assigned
    s_FLOAT,
    s_DOUBLE,
    s_LONGDOUBLE,
};

extern const char* int_types_str[];

struct number {
    union {
        unsigned long long integer;
        long double real;
    };
    enum int_types aux_type;
    bool is_signed;
};

struct strlit {
    char* str;
    size_t len;
};

#endif
