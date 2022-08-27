#include "ir.h"
#include "ir_arithmetic.h"
#include "ir_print.h"
#include "ir_state.h"
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


/**
 * Prepare target by allocating if necessary.
 * We don't always need a target, so this is split off
 * (e.g. ASTN_NUM).
 */
astn qprepare_target(astn target, astn qtype) {
    if (!target) {
        target = new_qtemp(qtype);
        qwarn("Allocated qtemp %d\n", target->Qtemp.tempno);
    } else {
        qwarn("Using existing target.");
    }

    return target;
}

astn get_qtype(const_astn t) {
    ir_type_E ret;

    switch (t->type) {
        case ASTN_NUM:
            switch (t->Num.number.aux_type) {
                case s_INT:
                    ret = IR_i32;
                    break;
                case s_LONG:
                    ret = IR_i64;
                    break;
                case s_CHARLIT:
                    ret = IR_i8;
                    break;
                default:
                    ret = IR_TYPE_UNDEF;
                    qunimpl(t, "Unsupported number literal int_type in IR:(");
            }

            return qtype_alloc(ret);

        case ASTN_SYMPTR:
            return get_qtype(t->Symptr.e->type);

        case ASTN_TYPE:
            if (t->Type.is_derived && t->Type.derived.type == t_PTR)
                ret = IR_ptr;

            switch (t->Type.scalar.type) {
                case t_INT:
                    ret = IR_i32;
                    break;
                case t_LONG:
                    ret = IR_i64;
                    break;
                case t_CHAR:
                    ret = IR_i8;
                    break;
                case t_SHORT:
                    ret = IR_i16;
                    break;
                default:
                    qunimpl(t, "Unsupported type in IR :(");
            }
            return qtype_alloc(ret);

        case ASTN_BINOP:
            // get resultant type
            // for now, just return i32 if it's arithmetic
            if (type_is_arithmetic(t))
                return qtype_alloc(IR_i32);

        default:
            qunimpl(t, "Unimplemented astn type in get_qtype :(");
    }

    die("Unreachable");
}

astn gen_rvalue(astn a, astn target) {
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

        default:
            qwarn("UH OH:\n");
            print_ast(a);
            die("Unhandled astn for gen_rvalue :(");
    }

    qunimpl(a, "Unimplemented astn in gen_rvalue :(");
}

void gen_quads(astn a) {
    switch (a->type) {
        case ASTN_RETURN:
            emit(IR_OP_RETURN, NULL, gen_rvalue(a->Return.ret, NULL), NULL);
            break;

        case ASTN_DECLREC:
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
            astn qtemp = new_qtemp(get_qtype(symptr_alloc(n)));

            emit(IR_OP_ALLOCA, qtemp, NULL, NULL);
        }

        n = n->next;
    }

    astn a = e->body;
    ast_check(a, ASTN_LIST, "");

    while (a && list_data(a)) {
        gen_quads(list_data(a));
        a = list_next(a);
    }
}

