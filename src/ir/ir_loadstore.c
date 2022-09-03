#include "ir_loadstore.h"

#include "ast.h"
#include "ir.h"
#include "ir_lvalue.h"
#include "ir_util.h"
#include "ir_types.h"
#include "lexer.h" // for print_context
#include "symtab.h"
#include "symtab_util.h"
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

    astn t = ir_dtype(lval); // underlying type

    if (ir_type_matches(t, IR_arr))
        qerror("Arrays are not assignable!")

    astn compat_rval = make_type_compat_with(rval, t);

    emit(IR_OP_STORE, lval, compat_rval, NULL);

    return compat_rval;
}

/*
 * Generate code for ASTN_ASSIGN. We return the rvalue
 * (right side).
 */
astn gen_assign(astn a) {
    if (a->type == ASTN_CASSIGN)
        return gen_rvalue(a, NULL);

    ast_check(a, ASTN_ASSIGN, "");

    return gen_store(a->Assign.left, a->Assign.right);
}

astn gen_select(astn a) {
    astn s_lval = gen_lvalue(a->Select.parent);

    if (!ir_type_matches(ir_dtype(s_lval), IR_struct))
        qerrorl(a, "Object is not a struct or union - cannot use member selection.");

    sym memb_e = st_lookup_fq(a->Select.member->Ident.ident, get_qtype(ir_dtype(s_lval))->Qtype.derived_type->Type.tagtype.symbol->members, NS_MEMBERS);

    if (!memb_e)
        qerrorl(a, "Ident is not a member of this struct.");

    astn target = qprepare_target(NULL, qtype_alloc(IR_ptr));
    target->Qtemp.qtype->Qtype.derived_type = get_qtype(symptr_alloc(memb_e));

    emit4(IR_OP_GEP, target, s_lval, simple_constant_alloc(0), simple_constant_alloc(memb_e->struct_offset));
    return target;
}
