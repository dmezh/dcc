#ifndef SYMTAB_H
#define SYMTAB_H

#include <stdbool.h>
#include "ast.h"

enum scope_types {
    SCOPE_FILE,
    SCOPE_FUNCTION,
    SCOPE_BLOCK,
    SCOPE_PROTOTYPE
};

enum namespaces {
    NS_TAGS,
    NS_LABELS,
    NS_MEMBERS,
    NS_MISC
};

typedef struct st_entry {
    char* ident;
    astn* type;
    enum namespaces ns;
    struct st_entry *next;
} st_entry;

typedef struct symtab {
    enum scope_types scope_type;
    struct symtab *parent;
    struct st_entry *first, *last;
} symtab;

st_entry* st_lookup(char* ident);
bool st_insert(char* ident);
void st_dump_single();

#endif
