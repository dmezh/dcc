#ifndef UTIL_H
#define UTIL_H

#include <stddef.h> // for size_t
#include <stdio.h> // for error macro below
#include <stdlib.h>

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

void *safe_malloc(size_t size);
void *safe_calloc(size_t num, size_t size);
void *safe_realloc(void* old, size_t size);
void die(const char* msg);

#endif
