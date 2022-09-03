#include "ir.h"
#include "ir_arithmetic.h"
#include "ir_loadstore.h"
#include "ir_lvalue.h"
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

static struct BB root_bb = {0};
static struct BBL root_bbl = {.me = &root_bb};

struct ir_state irst = {
    .root_bbl = &root_bbl,
    .bb = &root_bb,
};


static astn _gen_rvalue(astn a, astn target) {
    switch (a->type) {
        case ASTN_NUM:
            return a;

        case ASTN_STRLIT:
            return lvalue_to_rvalue(gen_anon(a), target);

        case ASTN_BINOP:
            switch (a->Binop.op) {
                case '+':
                    return gen_add_rvalue(a, target);
                case '-':
                    return gen_sub_rvalue(a, target);
                case ',':
                    gen_rvalue(a->Binop.left, NULL);
                    return gen_rvalue(a->Binop.right, target);
                default:
                    qunimpl(a, "Unhandled binop type for gen_rvalue :(");
            }

        case ASTN_UNOP:;
            astn prev;
            switch (a->Unop.op) {
                case '*':
                    return lvalue_to_rvalue(gen_indirection(a), target);

                case PLUSPLUS:;
                    prev = gen_rvalue(a->Unop.target, target);
                    gen_assign(cassign_alloc('+', gen_lvalue(a->Unop.target), simple_constant_alloc(1)));
                    return prev;

                case MINUSMINUS:;
                    prev = gen_rvalue(a->Unop.target, target);
                    gen_assign(cassign_alloc('-', gen_lvalue(a->Unop.target), simple_constant_alloc(1)));
                    return prev;

                case '&':;
                    return gen_lvalue(a->Unop.target);

                case PREINCR:
                    if (target)
                        die("Unexpected target.");

                    return gen_assign(cassign_alloc('+', gen_lvalue(a->Unop.target), simple_constant_alloc(1)));

                case PREDECR:
                    if (target)
                        die("Unexpected target.");

                    return gen_assign(cassign_alloc('-', gen_lvalue(a->Unop.target), simple_constant_alloc(1)));

                case '+':
                    if (target)
                        die("Unexpected target for unary +");

                    return do_integer_promotions(gen_rvalue(a->Unop.target, NULL));

                case '-':
                    if (target)
                        die("Unexpected target for unary -");

                    return do_negate(do_integer_promotions(gen_rvalue(a->Unop.target, NULL)));

                default:
                    qunimpl(a, "Unhandled unop in gen_rvalue :(");
            }

        case ASTN_ASSIGN:
            return gen_assign(a);

        case ASTN_CASSIGN:;
            astn assign = astn_alloc(ASTN_ASSIGN);
            astn bin = binop_alloc(a->Cassign.op, lvalue_to_rvalue(a->Cassign.left, NULL), a->Cassign.right);

            assign->Assign.left = a->Cassign.left;
            assign->Assign.right = bin;

            return gen_assign(assign);

        case ASTN_SYMPTR:
           return lvalue_to_rvalue(gen_lvalue(a), target);

        case ASTN_QTEMP:
            return a;

        default:
            qunimpl(a, "Unhandled astn for gen_rvalue :(");
    }

    qunimpl(a, "Unimplemented astn in gen_rvalue :(");
}

astn gen_rvalue(astn a, astn target) {
    astn r = _gen_rvalue(a, target);
    return r;
}

void gen_quads(astn a) {
    switch (a->type) {
        case ASTN_RETURN:;
            // TODO: check for void function type!
            astn retval = gen_rvalue(a->Return.ret, NULL);

            astn retval_conv = make_type_compat_with(retval, irst.fn->type->Type.derived.target);

            if (ir_type(retval_conv) != ir_type(irst.fn->type->Type.derived.target))
                qerrorl(a, "Return statement type does not match function return type");
            emit(IR_OP_RETURN, NULL, retval_conv, NULL);
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
        case ASTN_NUM:
        case ASTN_STRLIT:
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

// Allocate a qtemp for given anon thing, and add it to the list to define later
astn gen_anon(astn a) {
    astn qtype = qtype_alloc(IR_ptr);
    astn qtemp = qtemp_alloc(-1, qtype);

    astn dtype;

    switch (a->type) {
        case ASTN_STRLIT:;
            astn i8_tspec = typespec_alloc(TS_CHAR);
            astn i8_type = astn_alloc(ASTN_TYPE);

            describe_type(i8_tspec, &i8_type->Type);

            dtype = dtype_alloc(i8_type, t_ARRAY);
            dtype->Type.derived.size = simple_constant_alloc(a->Strlit.strlit.len + 1); // +1 for \0
            qtemp->Qtemp.global = a;
            qtemp->Qtemp.name = a->Strlit.strlit.str;

            break;

        default:
            qunimpl(a, "Invalid astn type for add_anon");
    }

    qtype->Qtype.derived_type = dtype;

    return qtemp;
}

void gen_global(sym e) {
    astn qtype = qtype_alloc(IR_ptr);

    qtype->Qtype.derived_type = e->type;

    astn qtemp = qtemp_alloc(-1, qtype);

    // double-link them to each other
    qtemp->Qtemp.global = symptr_alloc(e);
    e->ptr_qtemp = qtemp;

    qtemp->Qtemp.name = e->ident;

    emit(IR_OP_DEFGLOBAL, qtemp, NULL, NULL);
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
            emit(IR_OP_RETURN, NULL, gen_rvalue(simple_constant_alloc(0), NULL), NULL);
        }
    }
}

