#ifndef QUADS_PRINT_H
#define QUADS_PRINT_H

#include "quads.h"

#include <stdio.h>

extern char* quad_op_str[];

void print_node(const astn* qn);
void print_quad(quad* q);
void print_bbs();

#endif
