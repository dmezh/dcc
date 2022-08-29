#include "ir_util.h"

#include "ir.h"
#include "ir_state.h"

/**
 * Allocate and append new basic block to end of BBL.
 */
BB bb_alloc(void) {
    BB n = safe_calloc(1, sizeof(struct BB));
    n->bbno = irst.bb_count;
    n->fn = irst.fn;
    bbl_append(n);
    return n;
}

/**
 * Get next BBL of this BBL.
 */
BBL bbl_next(const BBL bbl) {
    return bbl->next;
}

/**
 * Get BB at this BBL.
 */
BB bbl_this(const BBL bbl) {
    return bbl->me;
}

/**
 * Append given BB to end of the global BBL.
 */
void bbl_append(BB bb) {
    BBL new = safe_calloc(1, sizeof(struct BBL));
    new->me = bb;

    // this is fucking dumb; keep a pointer to the end of the
    // global BBL chain in irst instead.
    BBL head = irst.root_bbl;
    while (head->next) head = head->next;

    head->next = new;
    return;
}

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

quad emit(ir_op_E op, astn target, astn src1, astn src2) {
    quad q = safe_calloc(1, sizeof(struct quad));

    *q = (struct quad){
        .op = op,
        .prev = 0,
        .next = 0,
        .target = target,
        .src1 = src1,
        .src2 = src2,
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
