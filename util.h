#ifndef UTIL_H
#define UTIL_H

#include <stddef.h> // for size_t
#include <stdio.h> // for error macro below

#define st_error(...) \
    fprintf(stderr, "Error declaring symbol: " __VA_ARGS__); \
    exit(-5);

void *safe_malloc(size_t size);
void *safe_calloc(size_t num, size_t size);
void *safe_realloc(void* old, size_t size);
void die(const char* msg);

#endif
