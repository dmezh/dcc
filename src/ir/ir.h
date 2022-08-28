#ifndef IR_H
#define IR_H

#include "ir_core.h"

#include "symtab.h"

extern struct BB bb_root;
extern BB current_bb;

astn qprepare_target(astn target, astn qtype);

astn get_qtype(const_astn t);

astn gen_rvalue(astn a, astn target);

ir_type_E type_to_ir(const_astn t);
void gen_fn(sym e);

#endif