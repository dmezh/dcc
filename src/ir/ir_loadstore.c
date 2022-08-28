#include "ir_loadstore.h"

#include "ast.h"
#include "ir.h"
#include "ir_arithmetic.h"
#include "ir_print.h"
#include "ir_util.h"
#include "lexer.h" // for print_context
#include "util.h"

astn gen_load(astn a, astn target) {
    // assuming local non-parameter variables right now.
    ast_check(a, ASTN_SYMPTR, "Expected symptr for gen_load!");

    target = qprepare_target(target, get_qtype(a));

    // we may need some more checking here?
    emit(IR_OP_LOAD, target, a->Symptr.e->ptr_qtemp, NULL);
    return target;
}

/*
 * Return address/object to write to.
 */
astn gen_lvalue(astn a) {
    switch (a->type) {
        case ASTN_SYMPTR:
            return a->Symptr.e->ptr_qtemp;

        case ASTN_NUM:
            qprintcontext(a->context);
            qerror("Expression is not assignable!");

        case ASTN_UNOP:
            if (a->Unop.op == '*')
                return gen_rvalue(a->Unop.target, NULL);

            qunimpl(a, "Unimplemented unop for gen_lvalue!");

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
