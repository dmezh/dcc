#include "util.h"

#include <features.h> // for __GLIBC__; I'm pretty sure stdio or something includes this anyway
#include <execinfo.h> // for backtrace
#include <stdio.h>
#include <stdlib.h>

void *safe_malloc(size_t size) {
    void *m = malloc(size);
    if (!m)
        die("Error allocating memory (malloc)");
    return m;
}

void *safe_calloc(size_t num, size_t size) {
    void *m = calloc(num, size);
    if (!m)
        die("Error allocating memory (calloc)");
    return m;
}

void *safe_realloc(void* old, size_t size) {
    void *m = realloc(old, size);
    if (!m)
        die("Error allocating memory (realloc)");
    return m;
}

/*
 * Die with backtrace
 *
 * backtrace() depends on glibc
 * some of this might fail depending on how much damage we did,
 * but we'll die in this function one way or another
 */
#define BACKTRACE_DEPTH 128

_Noreturn void die(const char* msg) {
    fprintf(stderr, "Internal error: %s\n", msg);

    #ifdef __GLIBC__
    fprintf(stderr, "Trying to print backtrace:\n------------------------------\n");
    void* callstack[BACKTRACE_DEPTH];
    int frames = backtrace(callstack, BACKTRACE_DEPTH);
    char** strings = backtrace_symbols(callstack, frames);
    for (int i = 0; i<frames; i++) {
        fprintf(stderr, "%s\n", strings[i]);
    }
    #endif

    abort();
}
