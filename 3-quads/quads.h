#ifndef QUADS_H
#define QUADS_H

#include "ast.h"
#include "symtab.h"

enum quad_op {
    Q_MOV,
    Q_ADD,
    Q_SUB,
    Q_MUL,
    Q_DIV,
    Q_MOD,
    Q_SHL,
    Q_SHR,
    Q_BWAND,
    Q_BWXOR,
    Q_BWOR,
    Q_LOAD,
    Q_STORE,
    Q_LEA
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
} BB;

extern unsigned temp_count;
extern BB* current_bb;

astn* qtemp_alloc(unsigned size);
void emit(enum quad_op op, astn* src1, astn* src2, astn* target);
void gen_quads(astn *n);
void todo(const char* msg);

#endif
