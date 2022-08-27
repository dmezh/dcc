#ifndef UTIL_H
#define UTIL_H

#include <stddef.h> // for size_t
#include <stdio.h> // for error macro below
#include <stdlib.h>

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

#define BRED "\033[1;31m"
#define RESET "\033[0m"

#define RED_ERROR(...) do {                         \
    eprintf(BRED);                          \
    eprintf(__VA_ARGS__);                   \
    eprintf(RESET "\n");                    \
    eprintf("\nCompilation failed :(\n");   \
    exit(-1);                                       \
} while(0);

void *safe_malloc(size_t size);
void *safe_calloc(size_t num, size_t size);
void *safe_realloc(void* old, size_t size);
_Noreturn __attribute((noreturn)) void die(const char* msg);

#endif
