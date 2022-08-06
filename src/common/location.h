#ifndef LOCATION_H
#define LOCATION_H

typedef struct YYLTYPE {
    char* filename;
    unsigned lineno;
} YYLTYPE;

#include "lexer.h"

// attribute needed to shut gcc up, but in general
// this probably shouldn't be in this header
static char* __attribute__((unused)) fnamestdin = "<stdin>";

#define YYLTYPE YYLTYPE
#define YYLLOC_DEFAULT(current, blah2, blah3) do { \
    (current) = context; \
    if (!(current).filename) (current).filename = fnamestdin; \
} while(0);

#endif
