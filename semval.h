#ifndef SEMVAL_H
#define SEMVAL_H

enum int_types {
    t_UNSPEC,
    t_CHARLIT,
    t_INT,
    t_LONG,
    t_LONGLONG,
    t_REAL, // just used for logic, shouldn't be assigned
    t_FLOAT,
    t_DOUBLE,
    t_LONGDOUBLE,
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
