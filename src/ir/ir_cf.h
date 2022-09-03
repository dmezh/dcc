#ifndef IR_CF_H
#define IR_CF_H

#include "ir.h"

BB bb_alloc(void);
BB bb_active(BB bb);
BB bbl_push(void);
void bbl_pop_to_root(void);

void uncond_branch(BB bb);

astn gen_equality_eq(astn a, astn b, astn target);
astn gen_equality_ne(astn a, astn b, astn target);
void gen_while(astn a);

#endif
