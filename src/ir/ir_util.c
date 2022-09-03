#include "ir_util.h"

#include "ir.h"
#include "ir_state.h"

/**
 * Allocate and return a new qtemp.
 */
astn new_qtemp(astn qtype) {
    return qtemp_alloc(++irst.tempno, qtype);
}

/**
 * Prepare target by allocating if necessary.
 * We don't always need a target, so this is split off
 * (e.g. ASTN_NUM).
 */
astn qprepare_target(astn target, astn qtype) {
    if (!target) {
        target = new_qtemp(qtype);
    } else {
        // left for debug
    }

    return target;
}

/**
 * Get last quad in basic block.
 */
quad last_in_bb(BB b) {
    quad q = b->current;
    if (!q) return NULL;

    while (q->next) q = q->next;

    return q;
}

quad emit4(ir_op_E op, astn target, astn src1, astn src2, astn src3) {
    quad q = safe_calloc(1, sizeof(struct quad));

    *q = (struct quad){
        .op = op,
        .prev = 0,
        .next = 0,
        .target = target,
        .src1 = src1,
        .src2 = src2,
        .src3 = src3,
    };

    if (!irst.bb->first) {
        irst.bb->first = q;
        irst.bb->current = q;
    } else {
        irst.bb->current->next = q;
        q->prev = irst.bb->current;
        irst.bb->current = q;
    }

    return q;
}

quad emit(ir_op_E op, astn target, astn src1, astn src2) {
    return emit4(op, target, src1, src2, NULL);
}
