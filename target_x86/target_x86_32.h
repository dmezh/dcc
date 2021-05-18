#ifndef TARGET_X86_32_H
#define TARGET_X86_32_H

#ifdef TARGET
#error "Define only one target!"
#endif
#define TARGET

#include "2-parser/types_common.h"

const int target_size[] = {
    [t_VOID]            = 4,
    [t_CHAR]            = 1,
    [t_SHORT]           = 2,
    [t_INT]             = 4,
    [t_LONG]            = 4,
    [t_LONGLONG]        = 8,
    [t_BOOL]            = 1,
    [t_REAL]            = 0,
    [t_FLOAT]           = 4,
    [t_DOUBLE]          = 8,
    [t_LONGDOUBLE]      = 12,
    [t_FLOATCPLX]       = 8,
    [t_DOUBLECPLX]      = 16,
    [t_LONGDOUBLECPLX]  = 24,
    [t_LASTSCALAR]      = 0,
    [t_PTR]             = 4,
    [t_ARRAY]           = 0,
    [t_FN]              = 4,
    [t_LASTDERIVED]     = 0,
    [t_UNION]           = 0,
    [t_STRUCT]          = 0
};

#endif