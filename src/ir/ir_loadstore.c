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

    if (type_is_arithmetic(a)) {    // simple load/store
        emit(IR_OP_LOAD, target, a->Symptr.e->ptr_qtemp, NULL);
    }
    return target;
}

astn gen_lvalue(astn a) {
    switch (a->type) {
        case ASTN_SYMPTR:
            return a->Symptr.e->ptr_qtemp;

        case ASTN_NUM:
            qprintcontext(a->context);
            qerror("Expression is not assignable!");

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
