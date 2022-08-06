/*
 * symtab_util.h
 *
 * Definitions for basic symbol table and scope stack tools.
 */

#ifndef SYMTAB_UTIL_H
#define SYMTAB_UTIL_H

#include "symtab.h"
#include "util.h"

#define st_error(...) \
    eprintf("Error declaring symbol: " __VA_ARGS__); \
    exit(-5);

sym stentry_alloc(const char *ident);

bool st_insert_given(sym new);

sym st_lookup(const char* ident, enum namespaces ns);
sym st_lookup_ns(const char* ident, enum namespaces ns);
sym st_lookup_fq(const char* ident, const symtab* s, enum namespaces ns);

void st_new_scope(enum scope_types scope_type, YYLTYPE openbrace_context);
void st_pop_scope();
void st_destroy(symtab* target);

#endif
