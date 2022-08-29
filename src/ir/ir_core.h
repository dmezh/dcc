#ifndef IR_CORE_H
#define IR_CORE_H

#include "ast.h"
#include "symtab.h"
#include "ir_defs.h"

struct astn_list;
struct quad {
    struct quad *prev;
    struct quad *next;

    ir_op_E op;

    astn target;
    astn src1;
    astn src2;
    astn src3;
};

typedef struct quad *quad;
typedef const struct quad *const_quad;

struct BB {
    quad first;
    quad current;

    int bbno;

    struct BB *prev;
    struct BB *next;

    sym fn;
};

typedef struct BB *BB;
typedef const struct BB *const_BB;

struct BBL {
    BB me;
    struct BBL *next;
};

typedef struct BBL *BBL;
typedef const struct BBL *const_BBL;

#endif
