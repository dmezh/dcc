#ifndef IR_CF_H
#define IR_CF_H

#include "ir.h"

BB bb_alloc(void);
BB bbl_push(void);
void bbl_pop_to_root(void);

void gen_while(astn a);

#endif
