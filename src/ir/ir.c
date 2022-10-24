#include "ir.h"
#include "ir_arithmetic.h"
#include "ir_cf.h"
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
    .current_bbl = &root_bbl,
    .bb = &root_bb,
};

astn gen_fncall(astn a, astn target) {
    astn arg = a->Fncall.args;
    astn arg_rval = NULL;

    while (arg) {
        if (!arg_rval)
            arg_rval = list_alloc(gen_rvalue(list_data(arg), NULL));
        else
            list_append(gen_rvalue(list_data(arg), NULL), arg_rval);
        arg = list_next(arg);
    }

    astn fn_ret = get_qtype(a->Fncall.fn->Symptr.e->type->Type.derived.target);
    if (ir_type_matches(fn_ret, IR_void)) {
        emit(IR_OP_FNCALL, NULL, a->Fncall.fn, arg_rval);
    } else {
        target = qprepare_target(target, fn_ret);
        emit(IR_OP_FNCALL, target, a->Fncall.fn, arg_rval);
    }

    return target;
}

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
                case EQEQ:
                    return gen_equality_eq(a->Binop.left, a->Binop.right, target);
                case NOTEQ:
                    return gen_equality_ne(a->Binop.left, a->Binop.right, target);
                case '<':
                case '>':
                case LTEQ:
                case GTEQ:
                    return gen_relational(a->Binop.left, a->Binop.right, a->Binop.op, target);
                case '*':
                case '%':
                case '/':
                    return gen_mulop_rvalue(a, target);

                case LOGAND:
                    return gen_logical_and(a, target);

                case LOGOR:
                    return gen_logical_or(a, target);

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

        case ASTN_FNCALL:
           return gen_fncall(a, target);

        case ASTN_SELECT:
           return lvalue_to_rvalue(gen_select(a), target);

        case ASTN_QTEMP:
            return a;

        case ASTN_TERN:
            return gen_ternary(a, target);

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
            if (!irst.fn)
                die("Not in function?");

            const bool fn_is_void = ir_type_matches(get_active_fn_target(), IR_void);

            if (fn_is_void) {
                if (a->Return.ret) {
                    qerrorl(a, "Void function should not return a value");
                } else {
                    break;
                }
            } else {
                if (!a->Return.ret) {
                    qerrorl(a, "Non-void function must return a value");
                }
            }

            astn retval = gen_rvalue(a->Return.ret, NULL);

            astn retval_conv = make_type_compat_with(retval, irst.fn->type->Type.derived.target);

            if (ir_type(retval_conv) != ir_type(irst.fn->type->Type.derived.target))
                qerrorl(a, "Return statement type does not match function return type");
            emit(IR_OP_RETURN, NULL, retval_conv, NULL);
            irst.tempno++;
            break;

        case ASTN_DECLREC:
            // generate initializers
            // assuming local scope
            if (a->Declrec.init && a->Declrec.e->storspec == SS_AUTO) {
                astn ass = astn_alloc(ASTN_ASSIGN);
                ass->Assign.left = symptr_alloc(a->Declrec.e);
                ass->Assign.right = a->Declrec.init;
                gen_assign(ass);
            }
            break;

        case ASTN_SELECT:
        case ASTN_BINOP:
        case ASTN_UNOP:
        case ASTN_NUM:
        case ASTN_STRLIT:
        case ASTN_SYMPTR:
            qwarn("Warning: useless expression: ");
            print_ast(a);
            gen_rvalue(a, NULL);
            break;

        case ASTN_FNCALL:
            gen_rvalue(a, NULL);
            break;

        case ASTN_BREAK:
            uncond_branch(irst.brk);
            irst.tempno++;
            break;

        case ASTN_CONTINUE:
            uncond_branch(irst.cont);
            irst.tempno++;
            break;

        case ASTN_WHILELOOP:
            if (a->Whileloop.is_dowhile)
                gen_dowhile(a);
            else
                gen_while(a);
            break;

        case ASTN_IFELSE:
            gen_if(a);
            break;

        case ASTN_FORLOOP:
            gen_for(a);
            break;

        case ASTN_SWITCH:
            gen_switch(a);
            break;

        case ASTN_NOOP:
            break;

        case ASTN_ASSIGN:
            gen_assign(a);
            break;

        case ASTN_CASSIGN:;
            astn assign = astn_alloc(ASTN_ASSIGN);
            astn bin = binop_alloc(a->Cassign.op, lvalue_to_rvalue(a->Cassign.left, NULL), a->Cassign.right);

            assign->Assign.left = a->Cassign.left;
            assign->Assign.right = bin;

            gen_assign(assign);
            break;

        case ASTN_LIST:
            while (a && list_data(a)) {
                gen_quads(list_data(a));
                a = list_next(a);
            }
            break;

        default:
            qunimpl(a, "Unimplemented astn for quad generation");
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
            asprintf(&qtemp->Qtemp.name, ".strlit.%s.%d", irst.fn->ident, irst.uniq++);

            if (irst.anons)
                list_append(qtemp, irst.anons);
            else
                irst.anons = list_alloc(qtemp);

            BB save = bb_jumproot();
            emit(IR_OP_DEFGLOBAL, qtemp, a, NULL);
            bb_active(save);

            break;

        default:
            qunimpl(a, "Invalid astn type for add_anon");
    }

    qtype->Qtype.derived_type = dtype;

    return qtemp;
}

void gen_global_named(sym e, const char *ident) {
    astn qtype;
    if (ir_type_matches(symptr_alloc(e), IR_fn))
        qtype = qtype_alloc(IR_fn);
    else
        qtype = qtype_alloc(IR_ptr);

    qtype->Qtype.derived_type = e->type;

    astn qtemp = qtemp_alloc(-1, qtype);

    // double-link them to each other
    qtemp->Qtemp.global = symptr_alloc(e);
    e->ptr_qtemp = qtemp;

    qtemp->Qtemp.name = strdup(ident);

    emit(IR_OP_DEFGLOBAL, qtemp, NULL, NULL);
}

void gen_global(sym e) {
    gen_global_named(e, e->ident);
}

static void gen_local(sym n) {
    if (n->entry_type == STE_VAR && n->storspec == SS_AUTO) {
        astn qtemp = new_qtemp(qtype_alloc(IR_ptr));
        qtemp->Qtemp.qtype->Qtype.derived_type = get_qtype(symptr_alloc(n));

        n->ptr_qtemp = qtemp;

        emit(IR_OP_ALLOCA, qtemp, NULL, NULL);
    } else if (n->entry_type == STE_VAR && n->storspec == SS_STATIC) {
        char *name;
        asprintf(&name, ".localstatic.%s.%s.%d", irst.fn->ident, n->ident, irst.uniq++);

        BB save = irst.bb;
        irst.bb = &root_bb;

        gen_global_named(n, name);

        irst.bb = save;
    } else if (n->type->Type.derived.type == t_FN) {
        char *name;
        asprintf(&name, "%s", n->ident);

        BB save = bb_jumproot();
        gen_global_named(n, name);
        irst.bb = save;
    }
}

static void gen_param(sym n) {
    if (n->entry_type == STE_VAR && n->storspec == SS_AUTO) {
        astn qtemp = new_qtemp(get_qtype(symptr_alloc(n)));

        n->param_qtemp = qtemp;

        if (!irst.fn->param_list_q)
            irst.fn->param_list_q = list_alloc(qtemp);
        else
            list_append(qtemp, irst.fn->param_list_q);
    }
}

void gen_fn(sym e) {
    irst.fn = e;

    irst.tempno = 0; // reset
    // irst.bb->bbno = 0;

    // generate parameters
    astn p = e->param_list;
    while (p) {
        astn a = list_data(p);
        if (a->type == ASTN_ELLIPSIS) {
            list_append(a, irst.fn->param_list_q);

            if (list_next(p))
                die("Unexpected list element past variadic ellipsis in gen_fn");

            break;
        }

        sym n = list_data(p)->Declrec.e;

        gen_param(n);

        p = list_next(p);
    }

    irst.bb = bbl_push();
    irst.tempno++;

    // generate parameters - memory
    p = e->param_list;
    while (p) {
        if (list_data(p)->type == ASTN_ELLIPSIS) {
            if (list_next(p))
                die("Unexpected list element past variadic ellipsis in gen_fn");

            break;
        }
        sym n = list_data(p)->Declrec.e;

        gen_local(n);

        p = list_next(p);
    }

    // check the all_sym list
    // this includes variables from sub-scopes :)
    astn l = e->fn_scope->all_syms;
    while (l) {
        sym n = list_data(l)->Declrec.e;

        gen_local(n);

        l = list_next(l);
    }

    // generate param -> memory stores
    p = e->param_list;
    while (p) {
       if (list_data(p)->type == ASTN_ELLIPSIS) {
            if (list_next(p))
                die("Unexpected list element past variadic ellipsis in gen_fn");

            break;
        }

        sym n = list_data(p)->Declrec.e;

        gen_store(n->ptr_qtemp, n->param_qtemp);

        p = list_next(p);
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

        if (ir_type_matches(get_active_fn_target(), IR_void)) {
            emit(IR_OP_RETURN, NULL, NULL, NULL);
        }
    }

    bbl_pop_to_root();
}

