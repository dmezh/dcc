#ifndef SYMTAB_PRINT_H
#define SYMTAB_PRINT_H

#include "symtab.h"

void st_dump_current(void);
void st_dump_entry(const_sym e);
void st_dump_recursive(void);
void st_dump_single_given(const symtab* s);

void st_examine_given(const_sym e);
void st_examine(const char* ident);
void st_examine_member(const char* tag, const char* child);

#endif
