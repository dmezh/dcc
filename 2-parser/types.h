#ifndef TYPES_H
#define TYPES_H

#include "ast.h"

// 6.7.1 - the below should include _Thread_local, but it has a different set of rules
// see 6.7.1 Constraints
enum storage_specs {
    // typedef,
    SPEC_AUTO = 0,
    SPEC_EXTERN,
    SPEC_STATIC,
    SPEC_REGISTER
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

// Note that 'int' may == either 'unsigned int' or 'signed int' in bitfields
// this may need to be changed
struct qualtype {
    enum scalar_types base_type;

    bool is_unsigned;

    bool is_volatile;
    bool is_const;
    bool is_restrict;
    bool is_atomic;

    enum storage_specs storage_spec;
};

#endif
