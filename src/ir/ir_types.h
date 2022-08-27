#ifndef IR_OPS_H
#define IR_OPS_H

typedef enum {
    IR_OP_UNKNOWN = 0,

    IR_OP_FNDECL,
    IR_OP_ALLOCA,
    IR_OP_LOAD,
    IR_OP_STORE,
    IR_OP_RETURN,

    IR_OP_ADD,

    IR_OP_COUNT,
} ir_op_E;

typedef enum {
    IR_TYPE_UNDEF = 0,

    IR_ptr,

    IR_i8,
    IR_i16,
    IR_i32,
    IR_i64,

    IR_TYPE_COUNT,
} ir_type_E;

static const char *ir_op_str[IR_OP_COUNT] = {
    [IR_OP_UNKNOWN] = 0,

    [IR_OP_FNDECL] = "fndecl",
    [IR_OP_ALLOCA] = "alloca",
    [IR_OP_LOAD] = "load",
    [IR_OP_STORE] = "store",
    [IR_OP_RETURN] = "ret",

    [IR_OP_ADD]    = "add",
};

static const char *ir_type_str[IR_TYPE_COUNT] = {
    [IR_TYPE_UNDEF] = 0,
    [IR_ptr] = "ptr",
    [IR_i8] = "i8",
    [IR_i16] = "i16",
    [IR_i32] = "i32",
    [IR_i64] = "i64",
};


#endif
