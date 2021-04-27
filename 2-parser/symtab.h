/*
 * symtab.h
 *
 * Core definitions for the symbol table.
 */

#ifndef SYMTAB_H
#define SYMTAB_H

#include <stdbool.h>

#include "ast.h"
#include "location.h"
#include "semval.h"
#include "types_common.h"

enum scope_types {
    SCOPE_MINI,
    SCOPE_FILE,
    SCOPE_FUNCTION,
    SCOPE_BLOCK,
    SCOPE_PROTOTYPE
};
extern const char* scope_types_str[];

enum namespaces {
    NS_TAGS,
    NS_LABELS,
    NS_MEMBERS, // I think this would be implied just by being in a struct/union def scope
    NS_MISC
};
extern const char* namespaces_str[];

/*
 * These will be directly pointed to by the AST.
 * The symbol table is a linked list of these.
*/
typedef struct st_entry {
    char *ident;
    enum namespaces ns;

    bool is_strunion_def;
    bool is_union; // tbd
    struct symtab *members; // tag type is complete when this is non-null

    struct astn *type;
    enum storspec storspec;
    bool has_init;
    union { // yeah we'll just copy initializers - avoids entangling us with the AST
        struct number numinit;
        struct strlit strinit;
    };
    
    struct st_entry *next;
    struct symtab* scope;
    YYLTYPE decl_context, def_context; // only some will have the latter
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
 *                                                             *current_scope 
 *    symtab 1               symtab 2              symtab 3  ⬋
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
    YYLTYPE context; // when the scope started
    struct symtab *parent;
    struct st_entry *first, *last;
} symtab;

extern symtab* current_scope;


st_entry *st_declare_struct(char* ident, bool strict,  YYLTYPE context);
st_entry *st_define_struct(char *ident, astn *decl_list, YYLTYPE context, YYLTYPE openbrace_context);

void begin_st_entry(astn *decl, enum namespaces ns,  YYLTYPE context);
st_entry* stentry_alloc(char *ident);

st_entry* st_lookup(const char* ident);
st_entry* st_lookup_ns(const char* ident, enum namespaces ns);
st_entry* st_lookup_fq(const char* ident, symtab* s, enum namespaces ns);

bool st_insert_given(st_entry *new);

void st_new_scope(enum scope_types scope_type, YYLTYPE openbrace_context);
void st_pop_scope();
void st_destroy(symtab* target);

void st_dump_entry(const st_entry* e);
void st_dump_single();
void st_dump_struct(st_entry* s);

void st_examine(char* ident);
void st_examine_member(char* tag, char* child);

#endif
