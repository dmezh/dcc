#include "ir_loadstore.h"

#include "ast.h"
#include "ir.h"
#include "ir_lvalue.h"
#include "ir_util.h"
#include "ir_types.h"
#include "lexer.h" // for print_context
#include "util.h"

/**
 * Load and return value.
 */
astn gen_load(astn a, astn target) {
    // assuming local non-parameter variables right now.
    astn addr = a;

    switch (a->type) {
        case ASTN_QTEMP:
            break;

        default:
            qunimpl(a, "Unexpected non-qtemp astn in gen_load!");
    }

    if (!ir_type_matches(addr, IR_ptr)) {
       qunimpl(addr, "Dereferenced non-pointer object!");
    }

    target = qprepare_target(target, get_qtype(ir_dtype(addr)));

    emit(IR_OP_LOAD, target, addr, NULL);
    return target;
}

/**
 * Store and return value.
 */
astn gen_store(astn target, astn val) {
    astn lval = gen_lvalue(target);
    astn rval = gen_rvalue(val, NULL);

    astn compat_rval = make_type_compat_with(rval, ir_dtype(lval));

    emit(IR_OP_STORE, lval, compat_rval, NULL);

    return rval;
}

/*
 * Generate code for ASTN_ASSIGN. We return the rvalue
 * (right side).
 * TODO: Compatibility?
 */
astn gen_assign(astn a) {
    ast_check(a, ASTN_ASSIGN, "");

    return gen_store(a->Assign.left, a->Assign.right);
}
