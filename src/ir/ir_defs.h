#ifndef IR_DEFS_H
#define IR_DEFS_H

#include <stddef.h>

typedef enum {
    IR_OP_UNKNOWN = 0,

    IR_OP_FNDECL,
    IR_OP_DEFGLOBAL,

    IR_OP_ALLOCA,
    IR_OP_LOAD,
    IR_OP_STORE,
    IR_OP_RETURN,
    IR_OP_GEP,

    IR_OP_FNCALL,
    IR_OP_BR,
    IR_OP_CONDBR,
    IR_OP_CMPEQ,
    IR_OP_CMPNE,
    IR_OP_CMPLT,
    IR_OP_CMPLTEQ,
    IR_OP_SWITCHBEGIN,
    IR_OP_SWITCHCASE,
    IR_OP_SWITCHEND,

    IR_OP_SEXT,
    IR_OP_ZEXT,
    IR_OP_TRUNC,
    IR_OP_INTTOPTR,
    IR_OP_PTRTOINT,

    IR_OP_ADD,
    IR_OP_SUB,
    IR_OP_MUL,
    IR_OP_SDIV,
    IR_OP_UDIV,
    IR_OP_SMOD,
    IR_OP_UMOD,

    IR_OP_COUNT,
} ir_op_E;

typedef enum {
    IR_TYPE_UNDEF = 0,

    IR_fn,
    IR_arr,
    IR_ptr,
    IR_void,
    IR_struct,
    IR_label,

    IR_TYPE_INTEGER_MIN,
    IR_i1,
    IR_u8,
    IR_i8,
    IR_u16,
    IR_i16,
    IR_u32,
    IR_i32,
    IR_u64,
    IR_i64,
    IR_TYPE_INTEGER_MAX,

    IR_TYPE_COUNT,
} ir_type_E;

static const char *ir_op_str[IR_OP_COUNT] = {
    [IR_OP_UNKNOWN] = 0,

    [IR_OP_FNDECL] = "fndecl",
    [IR_OP_DEFGLOBAL] = "global",

    [IR_OP_ALLOCA] = "alloca",
    [IR_OP_LOAD] = "load",
    [IR_OP_STORE] = "store",
    [IR_OP_RETURN] = "ret",
    [IR_OP_GEP] = "getelementptr",

    [IR_OP_FNCALL] = "call",
    [IR_OP_BR] = "br",

    [IR_OP_SEXT] = "sext",
    [IR_OP_ZEXT] = "zext",
    [IR_OP_TRUNC] = "trunc",
    [IR_OP_INTTOPTR] = "inttoptr",
    [IR_OP_PTRTOINT] = "ptrtoint",

    [IR_OP_ADD] = "add",
    [IR_OP_SUB] = "sub",
};

static const char *ir_type_str[IR_TYPE_COUNT] = {
    [IR_TYPE_UNDEF] = 0,
    [IR_ptr] = "ptr",
    [IR_void] = "void",
    [IR_label] = "label",
    [IR_i1] = "i1",
    [IR_u8] = "i8",
    [IR_i8] = "i8",
    [IR_u16] = "i16",
    [IR_i16] = "i16",
    [IR_u32] = "i32",
    [IR_i32] = "i32",
    [IR_u64] = "i64",
    [IR_i64] = "i64",
};

static size_t ir_type_size[IR_TYPE_COUNT] = {
    [IR_TYPE_UNDEF] = 0,
    [IR_ptr] = 8, // can get from target_size[]
    [IR_i1] = 1,
    [IR_u8] = 1,
    [IR_i8] = 1,
    [IR_u16] = 2,
    [IR_i16] = 2,
    [IR_u32] = 4,
    [IR_i32] = 4,
    [IR_u64] = 8,
    [IR_i64] = 8,
};

#define IR_PTR_INT_TYPE IR_i64

#endif
