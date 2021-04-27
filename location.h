#ifndef LOCATION_H
#define LOCATION_H

typedef struct YYLTYPE {
    char* filename;
    unsigned lineno;
} YYLTYPE;

#include "1-lexer/lexer.h"

static char* fnamestdin = "<stdin>";

#define YYLTYPE YYLTYPE
#define YYLLOC_DEFAULT(current, blah2, blah3) do { \
    (current) = context; \
    if (!(current).filename) (current).filename = fnamestdin; \
} while(0);

#endif
