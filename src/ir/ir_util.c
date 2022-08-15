#include "ir_util.h"

#include "ir.h"

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

    if (!current_bb->first) {
        current_bb->first = q;
        current_bb->current = q;
    } else {
        current_bb->current->next = q;
        q->prev = current_bb->current;
        current_bb->current = q;
    }

    return q;
}
