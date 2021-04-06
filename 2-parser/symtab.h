#ifndef SYMTAB_H
#define SYMTAB_H

#include <stdbool.h>

#include "ast.h"
#include "semval.h"

enum scope_types {
    SCOPE_FILE,
    SCOPE_FUNCTION,
    SCOPE_BLOCK,
    SCOPE_PROTOTYPE
};

enum namespaces {
    NS_TAGS,
    NS_LABELS,
    NS_MEMBERS, // I think this would be implied just by being in a struct/union def scope
    NS_MISC
};

typedef struct st_entry {
    char* ident;
    astn* type;
    enum namespaces ns;
    bool has_init;
    union { // yeah we'll just copy initializers - avoids entangling us with the AST
        struct number numinit;
        struct strlit strinit;
    };
    struct st_entry *next;
} st_entry;

/*
 * We will maintain a stack of lexical scopes using a simple linked list of symbol
 * tables. Each symbol table has a pointer to its parent - the scope from which
 * this scope was entered. We need this to properly resolve symbols during parsing,
 * where the lexical scope still matters. The root scope for the translation unit
 * is statically initialized and has a NULL *parent.
 * There's a global symtab* current_scope which is set first to the root. See symtab.c.
 *
 * Ex:                        stuff.c
 *                            ,------------------
 * current_scope: symtab 1   1| int thing1 = 0;   // install in symtab 1
 * current_scope: symtab 1   2| int thing2 = 1;   // install in symtab 1
 * current_scope: symtab 2   3| main()            // create symtab 2
 * current_scope: symtab 2   4| {
 * current_scope: symtab 2   5|   int thing1 = 1; // install in symtab 2
 * current_scope: symtab 2   6|   if (thing1)     // lookup matches from symtab 2
 * current_scope: symtab 3   7|   {               // create symtab 3
 * current_scope: symtab 3   8|       int thing3; // install in symtab 3
 * current_scope: symtab 3   9|       thing2++;   // lookup matches from symtab 1
 * current_scope: symtab 3  10|   }               // pop scope stack
 * current_scope: symtab 2  11|   thing3++;       // syntax error
 * current_scope: symtab 2  12| }                 // pop scope stack
 * current_scope: symtab 1  13|
 *
 *    symtab 1               symtab 2              symtab 3
 *   ,---------,  *parent  ,----------,  *parent  ,--------,
 *   | stuff.c |  <------  | main(){} |  <------  |  if{}  |
 *   `---------`           `----------`           `--------`
 *   last, you^    <---     then you^     <---       you^      | Symbol lookup order
 *
 * When we pop a scope we're setting current_scope to its parent, but we don't just lose
 * the popped scope/symtab - the AST still points to it.
 */
typedef struct symtab {
    enum scope_types scope_type;
    struct symtab *parent;
    struct st_entry *first, *last;
} symtab;

extern symtab* current_scope;

st_entry* st_lookup(const char* ident);
bool st_insert(char* ident);
void st_new_scope(enum scope_types scope_type);
void st_pop_scope();
void st_destroy(symtab* target);
void st_dump_single();

#endif
