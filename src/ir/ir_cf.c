#include "ir_cf.h"

#include "ir.h"
#include "ir_state.h"

BB bb_alloc(void) {
    BB new = safe_calloc(1, sizeof(struct BB));

    new->bbno = ++irst.tempno;
    new->fn = irst.fn;
    if (irst.bb_head) {
        irst.bb_head->next = new;
        new->prev = irst.bb_head;
        irst.bb_head = new;
    }
    return new;
}

BB bbl_push(void) {
    BBL new = safe_calloc(1, sizeof(struct BBL));
    if (irst.bb != irst.root_bbl->me)
        die("bbl_push should only be called to start a new function.");

    irst.current_bbl->next = new;
    irst.current_bbl = new;

    irst.bb = NULL;
    irst.bb_head = NULL;

    BB first = bb_alloc();

    irst.bb_head = first;

    new->me = first;
    irst.bb = first;

    return first;
}

void bbl_pop_to_root() {
    irst.bb = irst.root_bbl->me;
    irst.bb_head = irst.bb;
}

astn wrap_bb(BB bb) {
    astn a = astn_alloc(ASTN_QBB);
    a->Qbb.bb = bb;
    return a;
}

void uncond_branch(BB bb) {
    emit(IR_OP_BR, wrap_bb(bb), NULL, NULL);
}

void gen_while(astn wn) {
    struct astn_whileloop *w = &wn->Whileloop;

    //BB cond = bb_alloc();
    //BB body = bb_alloc();
    BB next = bb_alloc();

    //uncond_branch(cond);
    //uncond_branch(body);
    uncond_branch(next);

    irst.bb = next;
}
