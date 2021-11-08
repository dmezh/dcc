#ifndef QUADS_H
#define QUADS_H

#include "ast.h"
#include "symtab.h"

enum quad_op {
    Q_MOV, // done
    Q_ADD, // done
    Q_SUB, // done
    Q_MUL, // done
    Q_DIV, // done
    Q_MOD, // done
    Q_SHL, // skipq
    Q_SHR, // skipq
    Q_BWAND, // skipq
    Q_BWXOR, // skipq
    Q_BWOR, // skipq
    Q_LOAD, // done
    Q_STORE, // done
    Q_LEA, // done
    Q_CMP, // done
    Q_BREQ, // done
    Q_BRNE, // done
    Q_BRLT, // done
    Q_BRLE, // done
    Q_BRGT, // done
    Q_BRGE, // done
    Q_BR, // done
    Q_RET, // done
    Q_ARGBEGIN, // done
    Q_ARG, // done
    Q_CALL, // done
    Q_FNSTART, // done
    Q_NEG, // done
    Q_BWNOT // double check
};

enum qnode_types {
    QN_UNDEF,
    QN_ASTN,
    QN_STVAR,
    QN_CONSTANT,
    QN_TEMP
};

enum addr_modes {
    MODE_UNDEF,
    MODE_DIRECT,
    MODE_INDIRECT
};

typedef struct quad {
    enum quad_op op;
    struct quad *prev, *next;
    astn *target, *src1, *src2;
} quad;

typedef struct BB {
    struct BB *pred, *succ;
    quad *start, *cur;

    unsigned bbno;
    const char* fn;
    unsigned stack_offset_ez;
} BB;

typedef struct BBL {
    BB *me;
    struct BBL *next;
} BBL;

extern unsigned bb_count; // number of BB in this function
extern unsigned temp_count; // global temp var running count
extern BB* current_bb;

extern BBL bb_root;

BB* bb_alloc();

astn* qtemp_alloc(unsigned size);
astn* gen_rvalue(astn* node, astn* target);
void emit(enum quad_op op, astn* src1, astn* src2, astn* target);
void gen_fn(st_entry *e);
void gen_quads(astn *n);
void gen_assign(astn *node);
void todo(const char* msg);

#endif
