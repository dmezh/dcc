#ifndef IR_PRINT_H
#define IR_PRINT_H

#include "ast.h"
#include "ir.h"

#include <stdio.h>

const char *quad_astn_oneword_str(const_astn a);
void quad_print(quad first);
void quads_dump_llvm(FILE *o);

#endif
