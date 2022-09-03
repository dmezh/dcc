#include "ir_cf.h"

#include "ir.h"
#include "ir_arithmetic.h"
#include "ir_state.h"
#include "ir_types.h"
#include "ir_util.h"

#include "parser.tab.h"

BB bb_link(BB new) {
    new->fn = irst.fn;
    if (irst.bb_head) {
        irst.bb_head->next = new;
        new->prev = irst.bb_head;
        irst.bb_head = new;
    }
    return new;
}

BB bb_alloc(void) {
    BB new = safe_calloc(1, sizeof(struct BB));
    return new;
}

BB bb_nolink(const char *s) {
    BB new = bb_alloc();
    char *ss;
    asprintf(&ss, "%s.%d", s, irst.uniq++);
    new->name = ss;

    return new;
}

BB bb_named(const char* s) {
    return bb_link(bb_nolink(s));
}

BB bb_active(BB bb) {
    irst.bb = bb;
    return bb;
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
    bb_link(first);

    irst.bb_head = first;

    new->me = first;

    bb_active(first);

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

static void prepare_equality(astn a, astn b, astn *a_conv, astn *b_conv) {
    astn ar = gen_rvalue(a, NULL);
    astn br = gen_rvalue(b, NULL);

    bool a_is_arith = type_is_arithmetic(a);
    bool b_is_arith = type_is_arithmetic(b);

    if (a_is_arith && b_is_arith) {
        do_arithmetic_conversions(ar, br, a_conv, b_conv);
        return;
    }

    qunimpl(a, "Unimplemented operands in prepare_equality.");
}

void cmp0_br(astn a, BB t, BB f) {
    astn ar = gen_rvalue(a, NULL);

    astn res = gen_equality_eq(ar, simple_constant_alloc(0), NULL);
    emit(IR_OP_CONDBR, res, wrap_bb(t), wrap_bb(f));
}

astn gen_equality_eq(astn a, astn b, astn target) {
    astn a_conv;
    astn b_conv;

    prepare_equality(a, b, &a_conv, &b_conv);

    target = qprepare_target(target, qtype_alloc(IR_i1));
    emit(IR_OP_CMPEQ, target, a_conv, b_conv);

    return target;
}

astn gen_equality_ne(astn a, astn b, astn target) {
    astn a_conv;
    astn b_conv;

    prepare_equality(a, b, &a_conv, &b_conv);

    target = qprepare_target(target, qtype_alloc(IR_i1));
    emit(IR_OP_CMPNE, target, a_conv, b_conv);

    return target;
}

static void prepare_relational(astn a, astn b, astn *a_conv, astn *b_conv) {
    prepare_equality(a, b, a_conv, b_conv);
}

astn gen_relational(astn a, astn b, int op, astn target) {
    astn a_conv;
    astn b_conv;

    prepare_relational(a, b, &a_conv, &b_conv);

    target = qprepare_target(target, qtype_alloc(IR_i1));

    switch (op) {
        case '<':
            emit(IR_OP_CMPLT, target, a_conv, b_conv);
            break;
        case '>':
            emit(IR_OP_CMPLT, target, b_conv, a_conv);
            break;
        case GTEQ:
            emit(IR_OP_CMPLTEQ, target, a_conv, b_conv);
            break;
        case LTEQ:
            emit(IR_OP_CMPLTEQ, target, b_conv, a_conv);
            break;
        default:
            die("");
    }

    return target;
}

void uncond_branch(BB bb) {
    emit(IR_OP_BR, wrap_bb(bb), NULL, NULL);
}

void gen_if(astn ifnode) {
    struct astn_ifelse *ifn = &ifnode->Ifelse;

    BB thn = bb_named("if.then");
    BB els = bb_nolink("if.else");
    BB next = bb_nolink("if.fin");

    cmp0_br(ifn->condition_s, els, thn);

    bb_active(thn);
    bb_link(thn);
    gen_quads(ifn->then_s);
    uncond_branch(next);

    bb_active(els);
    bb_link(els);
    if (ifn->else_s)
        gen_quads(ifn->else_s);
    uncond_branch(next);

    bb_active(next);
    bb_link(next);
}

void gen_while(astn wn) {
    struct astn_whileloop *w = &wn->Whileloop;

    BB cond = bb_named("while.cond");
    BB body = bb_named("while.body");
    BB next = bb_named("while.next");

    uncond_branch(cond);

    bb_active(cond);
    cmp0_br(w->condition, next, body);

    bb_active(body);

    irst.brk = next;
    irst.cont = body;

    gen_quads(w->body);
    uncond_branch(cond);

    bb_active(next);
}

void gen_dowhile(astn dw) {
    struct astn_whileloop *d = &dw->Whileloop;

    BB cond = bb_named("dowhile.cond");
    BB body = bb_named("dowhile.body");
    BB next = bb_named("dowhile.next");

    uncond_branch(body);
    bb_active(body);

    irst.brk = next;
    irst.cont = body;

    gen_quads(d->body);

    uncond_branch(cond);
    bb_active(cond);

    cmp0_br(d->condition, next, body);

    bb_active(next);
}

void gen_for(astn fl) {
    struct astn_forloop *f = &fl->Forloop;

    BB cond = bb_named("for.cond");
    BB body = bb_named("for.body");
    BB next = bb_named("for.next");

    gen_quads(f->init);

    uncond_branch(cond);
    bb_active(cond);

    cmp0_br(f->condition, next, body);

    bb_active(body);

    irst.brk = next;
    irst.cont = body;

    gen_quads(f->body);
    gen_quads(f->oneach);

    uncond_branch(cond);

    bb_active(next);
}

