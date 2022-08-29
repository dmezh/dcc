#include "ir.h"
#include "ir_arithmetic.h"
#include "ir_loadstore.h"
#include "ir_state.h"
#include "ir_types.h"
#include "ir_util.h"

#include <string.h>

#include "ast.h"
#include "ast_print.h"
#include "parser.tab.h"
#include "symtab.h"
#include "types.h"
#include "util.h"

struct ir_state irst = {
    .root_bbl = &(struct BBL){0},
    .bb = &(struct BB){0},
};


astn ptr_target(astn a) {
    switch (a->type) {
        case ASTN_SYMPTR:
            return ptr_target(a->Symptr.e->type);

        case ASTN_TYPE:
            if (!a->Type.is_derived)
                qunimpl(a, "Non-derived type given to ptr_target!");

            return (a->Type.derived.target);

        case ASTN_UNOP:
            if (a->Unop.op != '*')
                qunimpl(a, "Non-derived type given to ptr_target!");

            return (a->Unop.target);

        default:
            qunimpl(a, "Unsupported astn in ptr_target :(");
    }
}
/*
static astn lvalue_to_rvalue(astn a, astn target) {
    // hoo wee
    switch (a->type) {
        case ASTN_SYMPTR:;
            astn t = get_qtype(a);
            if (t->Qtype.qtype == IR_arr) { // array lvalue to rvalue
                // assuming local storage?
                qprepare_target(target, get_qtype(t->Qtype.derived_type));
                emit(IR_OP_GEP, target, a,
            }
            break;

        default:
            die("");
    }
}
*/

static astn _gen_rvalue(astn a, astn target) {
    switch (a->type) {
        case ASTN_NUM:
            return a;

        case ASTN_BINOP:
            switch (a->Binop.op) {
                case '+':
                    return gen_add_rvalue(a, target);
                case '-':
                    return gen_sub_rvalue(a, target);
                default:
                    qwarn("UH OH:\n");
                    print_ast(a);
                    die("Unhandled binop type for gen_rvalue :(");
            }
            die("Unreachable");

        case ASTN_UNOP:
            switch (a->Unop.op) {
                case '*':
                    qwarn("Unop deref\n");
                    print_ast(a);
                    return gen_load(a, target);

                default:
                    qunimpl(a, "Unhandled unop in gen_rvalue :(");
            }

        case ASTN_SYMPTR: // decay array symbols to pointers.
            if (a->Symptr.e->type->Type.is_derived && a->Symptr.e->type->Type.derived.type == t_ARRAY)
                return try_decay(a);

            return gen_load(a, target);

        case ASTN_QTEMP:
            return a;

        default:
            qunimpl(a, "Unhandled astn for gen_rvalue :(");
    }

    qunimpl(a, "Unimplemented astn in gen_rvalue :(");
}

// 6.3.2.1.3
/*
    Except when it is the operand of the sizeof operator or the unary & operator, or is a
    string literal used to initialize an array, an expression that has type ‘‘array of type’’ is
    converted to an expression with type ‘‘pointer to type’’ that points to the initial element of
    the array object and is not an lvalue. If the array object has register storage class, the
    behavior is undefined.
*/

// try decay.
// The expression must have qtemp type.
astn try_decay(astn a) {
    astn t;

    switch (a->type) {
        case ASTN_QTEMP:
            t = a->Qtemp.qtype;
            break;

        case ASTN_SYMPTR:
            t = get_qtype(gen_lvalue(a))->Qtype.derived_type;
            a = gen_lvalue(a);
            qwarn("********** Got:");
            print_ast(t);
            break;

        default:
            return a;
    }

    if (t->Qtype.qtype != IR_arr)
        return a;

    qwarn("DECAYING\n");
    // change from (array of X) to (pointer to X)
    ast_check(t->Qtype.derived_type, ASTN_TYPE, "");

    if (!t->Qtype.derived_type->Type.is_derived)
        die("Expected derived type for qtemp with IR type arr!");

    astn targ = t->Qtype.derived_type;
    astn ptr = new_qtemp(qtype_alloc(IR_ptr));
    ptr->Qtemp.qtype->Qtype = (struct astn_qtype){
        .derived_type = targ,
        .qtype = IR_ptr,
    };
    //t->Qtype.derived_type = targ;
    //t->Qtype.qtype = IR_ptr;
    emit4(IR_OP_GEP, ptr, a, simple_constant_alloc(0), simple_constant_alloc(0));

    return ptr;
}
*/
astn gen_rvalue(astn a, astn target) {
    astn r = _gen_rvalue(a, target);
    qwarn("Before decay:\n");
    if (r->type == ASTN_QTEMP && r->Qtemp.qtype->Qtype.derived_type)
        print_ast(r->Qtemp.qtype->Qtype.derived_type), print_ast(r);
    r = try_decay(r);
    qwarn("After decay:\n");
    if (r->type == ASTN_QTEMP && r->Qtemp.qtype->Qtype.derived_type)
        print_ast(r->Qtemp.qtype->Qtype.derived_type), print_ast(r);
    return r;
}

void gen_quads(astn a) {
    switch (a->type) {
        case ASTN_RETURN:
            emit(IR_OP_RETURN, NULL, gen_rvalue(a->Return.ret, NULL), NULL);
            break;

        case ASTN_DECLREC:
            // generate initializers
            // assuming local scope
            if (a->Declrec.init) {
                astn ass = astn_alloc(ASTN_ASSIGN);
                ass->Assign.left = symptr_alloc(a->Declrec.e);
                ass->Assign.right = a->Declrec.init;
                gen_assign(ass);
            }
            break;

        case ASTN_BINOP:
        case ASTN_UNOP:
            qwarn("Warning: useless expression: ");
            print_ast(a);
            gen_rvalue(a, NULL);
            break;

        case ASTN_FNCALL:
            gen_rvalue(a, NULL);
            break;

        case ASTN_NOOP:
            break;

        case ASTN_ASSIGN:
            gen_assign(a);
            break;

        default:
            print_ast(a);
            die("Unimplemented astn for quad generation");
    }
}

void gen_fn(sym e) {
    irst.fn = e;
    irst.bb = bb_alloc();

    // generate local allocations
    sym n = e->fn_scope->first;
    while (n) {
        if (n->entry_type == STE_VAR && n->storspec == SS_AUTO) {
            astn qtemp = new_qtemp(qtype_alloc(IR_ptr));
            qtemp->Qtemp.qtype->Qtype.derived_type = get_qtype(symptr_alloc(n));
            n->ptr_qtemp = qtemp;

            emit(IR_OP_ALLOCA, qtemp, NULL, NULL);
        }

        n = n->next;
    }

    astn a = e->body;
    ast_check(a, ASTN_LIST, "");

    // generate body quads
    while (a && list_data(a)) {
        gen_quads(list_data(a));
        a = list_next(a);
    }

    // check return - should check non-main too
    const_quad const last = last_in_bb(irst.bb);
    if (!last || last->op != IR_OP_RETURN) {
        if (!strcmp(irst.fn->ident, "main")) {
            qwarn("Detected implicit return\n");
            emit(IR_OP_RETURN, NULL, gen_rvalue(simple_constant_alloc(0), NULL), NULL);
        }
    }
}

