#ifndef IR_PRINT_H
#define IR_PRINT_H

#include "ast.h"
#include "ir.h"

const char *qoneword(astn a);
void quad_print(quad first);
void quad_print_blankline(void);
void quads_dump_llvm(FILE *o);

#endif
