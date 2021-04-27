/*
 * types_common.h
 *
 * Common definitions relating to types.
 */

#ifndef TYPES_COMMON_H
#define TYPES_COMMON_H

// all of the below type-related shit should be moved out of ast.h
// these are type SPECIFIERS, NOT types
enum typespec {
    TS_VOID,
    TS_CHAR,
    TS_SHORT,
    TS_INT,
    TS_LONG,
    TS_FLOAT,
    TS_DOUBLE,
    TS_SIGNED,
    TS_UNSIGNED,
    TS__BOOL,
    TS__COMPLEX,
};

enum typequal {
    TQ_CONST,
    TQ_RESTRICT,
    TQ_VOLATILE
};

enum storspec {
    SS_AUTO = 0,
    SS_TYPEDEF, // we're not doing typedefs, just ignore, plus this probably shouldn't be here
    SS_EXTERN,
    SS_STATIC,
    SS_REGISTER
};

// standard defines "scalar types" differently, I don't care;
// I just couldn't find another word to describe this enum
enum scalar_types {
    SCALAR_TYPE_UNSPEC = 0,
    t_VOID,
    t_CHAR,
    t_SHORT,
    t_INT,
    t_LONG,
    t_LONGLONG,
    t_BOOL,
    t_REAL, // just used for logic, shouldn't be assigned
    t_FLOAT,
    t_DOUBLE,
    t_LONGDOUBLE,
    t_FLOATCPLX,
    t_DOUBLECPLX,
    t_LONGDOUBLECPLX
};

enum der_types {
    t_PTR,
    t_ARRAY,
    t_FN,
};

extern const char* storspec_str[];
extern const char* der_types_str[];

#endif
