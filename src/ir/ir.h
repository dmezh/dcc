#ifndef IR_H
#define IR_H

#include "ir_core.h"

#include "symtab.h"

extern struct BB bb_root;
extern BB current_bb;

astn gen_rvalue(astn a, astn target);

void gen_quads(astn a);

astn gen_anon(astn a);
void gen_global(sym e);
void gen_fn(sym e);

#endif
