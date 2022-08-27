#ifndef IR_UTIL_H
#define IR_UTIL_H

#include "ast.h"
#include "ir.h"

BB bb_alloc(void);
BBL bbl_next(const BBL bbl);
BB bbl_this(const BBL bbl);
void bbl_append(BB bb);

astn new_qtemp(astn qtype);
quad last_in_bb(BB bb);

quad emit(ir_op_E op, astn target, astn src1, astn src2);

#endif
