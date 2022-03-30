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
    SCOPE_UNDEF = 0,
    SCOPE_MINI,
    SCOPE_FILE,
    SCOPE_FUNCTION,
    SCOPE_BLOCK,
    SCOPE_PROTOTYPE
};

enum namespaces {
    NS_UNDEF = 0,
    NS_TEMP_INTERNAL,
    NS_TAGS,
    NS_LABELS,
    NS_MEMBERS, // I think this would be implied just by being in a struct/union def scope
    NS_MISC
};

enum st_entry_types {
    STE_UNDEF = 0,
    STE_VAR,
    STE_STRUNION_DEF,
    STE_FN
};

enum st_linkage {
    L_UNDEF = 0,
    L_NONE,
    L_INTERNAL,
    L_EXTERNAL
};


/*
 * These will be directly pointed to by the AST.
 * The symbol table is a linked list of these.
*/
typedef struct st_entry {
    enum st_entry_types entry_type;

    // struct/union
    bool is_union;
    struct symtab* members; // tag type is complete when this is non-null

    // var
    enum storspec storspec;
    bool is_param;
    bool has_init;
    union { // yeah we'll just copy initializers - avoids entangling us with the AST
        struct number numinit;
        struct strlit strinit;
    };
    int stack_offset;

    // fn
    astn param_list;
    struct symtab* fn_scope;
    astn body;
    bool fn_defined;
    bool variadic;

    const char *ident;
    enum namespaces ns;
    enum st_linkage linkage;
    astn type;
    struct st_entry *next;
    struct symtab* scope;
    YYLTYPE decl_context, def_context;
} st_entry;

typedef st_entry* sym;
typedef const st_entry* const_sym;

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
    int stack_total, param_stack_total;
    YYLTYPE context; // when the scope started
    struct symtab *parent;
    sym parent_func;
    sym first, last;
} symtab;

extern symtab root_symtab;
extern symtab* current_scope;

void st_reserve_stack(sym e);

sym st_define_function(astn fndef, astn block, YYLTYPE openbrace_context);
sym st_declare_function(astn fndef, YYLTYPE openbrace_context);

sym st_declare_struct(const char* ident, bool strict,  YYLTYPE context);
sym st_define_struct(const char *ident, astn decl_list,
                           YYLTYPE name_context, YYLTYPE closebrace_context, YYLTYPE openbrace_context);

sym begin_st_entry(astn decl, enum namespaces ns,  YYLTYPE context);

#endif
