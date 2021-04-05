#include <stdlib.h>

void *safe_malloc(size_t size);
void *safe_calloc(size_t num, size_t size);
void *safe_realloc(void* old, size_t size);
void die(char* msg);