#ifndef QUADS_PRINT_H
#define QUADS_PRINT_H

#include "quads.h"

#include <stdio.h>

extern char* quad_op_str[];

void print_node(const_astn qn, FILE* f);
void print_quad(const quad* q, FILE* f);
void print_bbs(FILE* f);

#endif
