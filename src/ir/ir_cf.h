#ifndef IR_CF_H
#define IR_CF_H

#include "ir.h"

BB bb_alloc(void);
BB bbl_push(void);
void bbl_pop_to_root(void);

astn gen_equality_eq(astn a, astn b, astn target);
void gen_while(astn a);

#endif
