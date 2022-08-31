#include "ir_loadstore.h"

#include "ast.h"
#include "ir.h"
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

    emit(IR_OP_STORE, lval, rval, NULL);

    return rval;
}

/**
 * Get lvalue.
 */
astn gen_lvalue(astn a) {
    switch (a->type) {
        case ASTN_SYMPTR:;
            astn n = a->Symptr.e->ptr_qtemp;
            return n;

        case ASTN_NUM:
            qprintcontext(a->context);
            qerror("Expression is not assignable!");

        case ASTN_UNOP:
            return gen_indirection(a);

        case ASTN_QTEMP:
            return a;

        default:
            qunimpl(a, "Unimplemented astn kind for gen_lvalue!");
    }
}

/**
 * Generate indirection lvalue (unop *).
 */
astn gen_indirection(astn a) {
    ast_check(a, ASTN_UNOP, "");
    if (a->Unop.op != '*')
        die("Passed wrong unop type to gen_indirection");

    astn targ_rval = gen_rvalue(a->Unop.target, NULL);

    if (!ir_type_matches(targ_rval, IR_ptr))
        qerror("Object to be dereferenced is not a pointer.");

    // if the operand points to a function, the result is a function designator.
    // TODO

    // if it points to an object, the result is an lvalue designating the object.
    return gen_lvalue(targ_rval);
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
