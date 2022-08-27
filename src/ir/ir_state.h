#ifndef IR_STATE_H
#define IR_STATE_H

#include "ir.h"
#include "symtab.h"

struct ir_state {
    // current function
    sym fn;

    // cursor for break/continue
    BB brk; BB cont;

    // total number of basic blocks
    int bb_count;

    // root BBL
    BBL root_bbl;

    // current BB
    BB bb;

    // total number of qtemps
    int tempno;
};

extern struct ir_state irst;

#endif
