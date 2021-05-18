#ifndef ASMGEN_H
#define ASMGEN_H

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
    enum asm_dirs dir;
    int constant;
} adir;

void asmgen();

#endif
