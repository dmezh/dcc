#include "ir.h"
#include "ir_arithmetic.h"
#include "ir_loadstore.h"
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
    } else {
        // left for debug
    }

    return target;
}

astn get_qtype(const_astn t) {
    ir_type_E ret;
    astn ret_der = NULL;

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

        case ASTN_SYMPTR:;
            astn qt = get_qtype(t->Symptr.e->type);
            // qt->Qtype.derived_type = t->Symptr.e->type;
            return qt;

        case ASTN_TYPE:
            if (t->Type.is_derived && t->Type.derived.type == t_PTR) {
                ret = IR_ptr;
                ret_der = t->Type.derived.target;
            } else {
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
            }
            astn q = qtype_alloc(ret);
            q->Qtype.derived_type = ret_der;
            return q;

        case ASTN_BINOP:
            // get resultant type
            // for now, just return i32 if it's arithmetic
            if (type_is_arithmetic(t))
                return qtype_alloc(IR_i32);
            else
                qunimpl(t, "Haven't implemented non-arithmetic binops");

        case ASTN_QTEMP:
            return t->Qtemp.qtype;

        case ASTN_UNOP:; // the type of a unop deref is special.
            astn utarget = t->Unop.target;

            if (t->Unop.op != '*')
                qunimpl(t, "Unsupported unop type in get_qtype");

            // HOO BOY
            // let's construct a qtype that has the actual derived chain.
            // we'll need to go down and get the following type,
            // and then when we eventually get to the base type or qtemp,
            // recursion stops.

            // int **i;
            // deref                       -> INT
            //   deref                     -> PTR to INT
            //     PTR to PTR to INT

            switch (utarget->type) {
                case ASTN_SYMPTR:;
                    // the target is a variable.
                    // Say we had int *i;
                    //            *i;
                    //
                    // The AST is:
                    // UNOP DEREF
                    //    sym i
                    //
                    // The type of i is:
                    // PTR TO
                    //    int

                    // so where if we just got i we'd go get_qtype(i), now
                    // we want get_qtype(i->derived target), which is int.
                    // So let's get the type of i as usual and then see.
                    astn n = get_qtype(utarget); // should be ptr, PTR TO int
                    if (n->Qtype.derived_type) {
                        // there's a derived type! get the next.
                        ast_check(n->Qtype.derived_type, ASTN_TYPE, "");
                        qwarn("True bottom of chain, printing type\n");
                        print_ast(n->Qtype.derived_type);
                        return get_qtype(n->Qtype.derived_type);
                    } else {
                        qerror("Dereferenced non-pointer symbol!");
                    }
                    break;

                case ASTN_UNOP:;
                    // Say we have
                    // deref
                    //   deref
                    //     sym i
                    //
                    // and i has:
                    // PTR TO
                    //   PTR TO
                    //     INT
                    //
                    // top deref should see another deref below it.
                    // get its type. Then, deref that. That's our type.
                    // If it's not a derived type, we ran out of stuff
                    // in the type chain and this is not a valid indirection.
                    astn next_deref_qtype = get_qtype(utarget); // recurse and get end
                        //qunimpl(t, "Fell off the end of derived type chain.");
                    qwarn("UNOP level, type before: %s\n", ir_type_str[next_deref_qtype->Qtype.qtype]);
                    print_ast(next_deref_qtype->Qtype.derived_type);
                    if (next_deref_qtype->Qtype.derived_type->Type.is_derived)
                        next_deref_qtype->Qtype.derived_type = next_deref_qtype->Qtype.derived_type->Type.derived.target;
                    else // not derived!
                        next_deref_qtype = get_qtype(next_deref_qtype->Qtype.derived_type);

                    qwarn("UNOP level, type after: %s\n", ir_type_str[next_deref_qtype->Qtype.qtype]);
                    print_ast(next_deref_qtype->Qtype.derived_type);
                    return next_deref_qtype;
                    //}

                    // what the hell are we doing peeking into the NEXT NEXT unop??
/*
                    if (!next_deref_qtype->Qtype.derived_type->Type.is_derived) {
                        qwarn("GOT HERE!!!!!!!!!!!!!!!!!!!!!!\n");
                        print_ast(next_deref_qtype->Qtype.derived_type);
                        return next_deref_qtype;
                    } else {
                        qunimpl(next_deref_qtype->Qtype.derived_type, "Failed to get to bottom of derived type chain.");
                    }
                    */

                    //astn this_deref_qtype = get_qtype(next_deref_qtype->Qtype.derived_type);

                   //  return this_deref_qtype;

                    /*
                    if (q->Qtype.derived_type)
                        return get_qtype(q->Qtype.derived_type);
                    else
                        return q;
                    */


                default:
                    qunimpl(utarget, "Invalid target of unop deref in get_qtype");
            }

        default:
            qunimpl(t, "Unimplemented astn type in get_qtype :(");
    }

    die("Unreachable");
}

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

astn gen_rvalue(astn a, astn target) {
    qwarn("Doing gen_rvalue\n");

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
                    return gen_load(a, target);

                default:
                    qunimpl(a, "Unhandled unop in gen_rvalue :(");
            }

        case ASTN_SYMPTR: // loading!
            return gen_load(a, target);

        case ASTN_QTEMP:
            return a;

        default:
            qunimpl(a, "Unhandled astn for gen_rvalue :(");
    }

    qunimpl(a, "Unimplemented astn in gen_rvalue :(");
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
            astn qtemp = new_qtemp(get_qtype(symptr_alloc(n)));
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

