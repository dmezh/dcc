#ifndef IR_CORE_H
#define IR_CORE_H

#include "ast.h"
#include "symtab.h"
#include "ir_types.h"

struct astn_list;
struct quad {
    struct quad *prev;
    struct quad *next;

    ir_op_E op;

    astn target;
    astn src1;
    astn src2;
};

typedef struct quad *quad;

struct BB {
    quad first;
    quad current;

    int bbno;

    struct BB *prev;
    struct BB *next;

    sym fn;
};

typedef struct BB *BB;

#endif
