#include "ir_loadstore.h"

#include "ast.h"
#include "ir.h"
#include "ir_arithmetic.h"
#include "ir_util.h"
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
    ast_check(a, ASTN_SYMPTR, "Can only lvalue a symptr rn");
    return a->Symptr.e->ptr_qtemp;
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
