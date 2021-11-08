#ifndef ASMGEN_H
#define ASMGEN_H

#include "quads.h"

typedef enum asm_dirs {
    NONE,
    CONSTANT,
    EBP,
    ESP,
    PUSHL,
    MOVL,
    SUBL,
    LEAVE
} adir_q;

typedef struct adir {
    enum asm_dirs d;
    int c;
} adir;

void e_cbr(const char *op, quad* q);
void e_bba(const astn *n);
void e_bb(const BB* b);
void asmgen(const BBL* bbl, FILE* out);

#endif
