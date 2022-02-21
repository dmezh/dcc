/*
 * symtab_util.h
 *
 * Definitions for basic symbol table and scope stack tools.
 */

#ifndef SYMTAB_UTIL_H
#define SYMTAB_UTIL_H

#include "symtab.h"

st_entry* stentry_alloc(const char *ident);

bool st_insert_given(st_entry *new);

st_entry* st_lookup(const char* ident, enum namespaces ns);
st_entry* st_lookup_ns(const char* ident, enum namespaces ns);
st_entry* st_lookup_fq(const char* ident, const symtab* s, enum namespaces ns);

void st_new_scope(enum scope_types scope_type, YYLTYPE openbrace_context);
void st_pop_scope();
void st_destroy(symtab* target);

#endif
