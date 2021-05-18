#ifndef ASMGEN_H
#define ASMGEN_H

#include "../3-quads/quads.h"

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

void e_cbr(char *op, quad* q);
void e_bba(astn *n);
void e_bb(BB* b);
void asmgen(BBL* bbl);

#endif
