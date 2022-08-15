/*
 * util.c
 *
 * Project-wide utilities
 */
#include "util.h"

// #include <features.h> // for __GLIBC__; I'm pretty sure stdio or something includes this anyway
#include <execinfo.h> // for backtrace
#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "yak.ascii.h"

/* 
 * -------------------------------------------------------------------
 * Safe memory allocation - always use these and not raw malloc(), etc
 * -------------------------------------------------------------------
 */
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
 * -------------------------------------------------------------------
 */


/*
 * Die with backtrace, for internal errors
 *
 * backtrace() depends on glibc
 * some of this might fail depending on how much damage we did,
 * but we'll die in this function one way or another
 */
#define BACKTRACE_DEPTH 128

_Noreturn __attribute__((noreturn)) void die(const char* msg) {
    eprintf("\nInternal error: %s\n", msg);

    eprintf("%s\n", yak);

    // #ifdef __GLIBC__
    eprintf("Trying to print backtrace:\n------------------------------\n");
    if (dcc_is_host_darwin()) {
        void __sanitizer_print_stack_trace(void);
        __sanitizer_print_stack_trace();
    } else {
        void* callstack[BACKTRACE_DEPTH];
        int frames = backtrace(callstack, BACKTRACE_DEPTH);
        char** strings = backtrace_symbols(callstack, frames);
        for (int i = 0; i<frames; i++) {
            eprintf("%s\n", strings[i]);
        }
    }
    // #endif

    abort();
}
