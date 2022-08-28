#include "ir_loadstore.h"

#include "ast.h"
#include "ir.h"
#include "ir_arithmetic.h"
#include "ir_print.h"
#include "ir_util.h"
#include "ir_state.h"
#include "lexer.h" // for print_context
#include "util.h"

/**
 * Load and return value.
 */
astn gen_load(astn a, astn target) {
    qwarn("DOING gen_load -----------------\n");
    // assuming local non-parameter variables right now.
    astn addr = NULL;

    switch (a->type) {
        case ASTN_SYMPTR:
            // addr = a->Symptr.e->ptr_qtemp;
            addr = gen_lvalue(a);
            qwarn("Made symptr %d:\n", irst.tempno);
            break;

        case ASTN_UNOP:
            addr = gen_lvalue(a);
            // addr->Qtemp.qtype = get_qtype(a);
            qwarn("Made lvalue %d with type:\n", irst.tempno);
            // print_ast(addr->Qtemp.qtype->Qtype.derived_type);
            break;

        default:
            qunimpl(a, "Bizarre type to try to load...");
    }

    qwarn("CONT gen_load -----------------\n");

    target = qprepare_target(target, get_qtype(a));
    qwarn("gen_load: Target type is %s, a follows\n", ir_type_str[target->Qtemp.qtype->Qtype.qtype]);
    print_ast(target->Qtemp.qtype->Qtype.derived_type);

    qwarn("Made it past debug.\n");

    // may need revision for globals
    ast_check(addr, ASTN_QTEMP, "Expected qtemp for loading!");
    emit(IR_OP_LOAD, target, addr, NULL);
    return target;
}

/*
 * Return address/object to write to.
 */
astn gen_lvalue(astn a) {
    switch (a->type) {
        case ASTN_SYMPTR:;
            astn n = a->Symptr.e->ptr_qtemp;
            n->Qtemp.qtype = get_qtype(n);
            return n;

        case ASTN_NUM:
            qprintcontext(a->context);
            qerror("Expression is not assignable!");

        case ASTN_UNOP:;
            if (a->Unop.op != '*')
                qunimpl(a, "Unimplemented unop for gen_lvalue!");

            astn l = gen_rvalue(a->Unop.target, NULL); // get the rvalue of the target
            // l->Qtemp.qtype = get_qtype(a);
            return l;

        default:
            qunimpl(a, "Unimplemented astn kind for gen_lvalue!");
    }
}

astn gen_store(astn target, astn val) {
    astn lval = gen_lvalue(target);
    astn rval = gen_rvalue(val, NULL);

    emit(IR_OP_STORE, lval, rval, NULL);

    return rval;
}

/*
 * Generate code for ASTN_ASSIGN. We return the rvalue
 * (right side).
 */
astn gen_assign(astn a) {
    ast_check(a, ASTN_ASSIGN, "");

    return gen_store(a->Assign.left, a->Assign.right);
}
