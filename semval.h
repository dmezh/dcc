#ifndef SEMVAL_H
#define SEMVAL_H

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

static const char* int_types_str[] = {
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

struct number {
    unsigned long long integer;
    long double real;
    int aux_type;
    int is_signed;
};

struct strlit {
    char* str;
    int len;
};

#endif
