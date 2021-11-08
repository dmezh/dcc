#ifndef SYMTAB_PRINT_H
#define SYMTAB_PRINT_H

#include "symtab.h"

void st_dump_entry(const st_entry* e);
void st_dump_single();
void st_dump_struct(const st_entry* s);

void st_examine(const char* ident);
void st_examine_member(const char* tag, const char* child);

void st_dump_recursive(void);

#endif
