#ifndef QUADS_PRINT_H
#define QUADS_PRINT_H

#include "quads.h"

#include <stdio.h>

extern char* quad_op_str[];

void print_node(const astn* qn, FILE* f);
void print_quad(quad* q, FILE* f);
void print_bbs(FILE* f);

#endif
