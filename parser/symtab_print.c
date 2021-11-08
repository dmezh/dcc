#include "symtab_print.h"

#include <stdio.h>

#include "ast_print.h"
#include "symtab.h"

static void st_dump_single_given(const symtab* s);

static const char* scope_types_str[] = {
    [SCOPE_MINI] = "MINI",
    [SCOPE_FILE] = "GLOBAL",
    [SCOPE_FUNCTION] = "FUNCTION",
    [SCOPE_BLOCK] = "BLOCK",
    [SCOPE_PROTOTYPE] = "PROTOTYPE"
};

static const char* namespaces_str[] = {
    [NS_TAGS] = "TAGS",
    [NS_LABELS] = "LABELS",
    [NS_MEMBERS] = "MEMBERS",
    [NS_MISC] = "MISC"
};

static const char* st_entry_types_str[] = {
    [STE_UNDEF] = "UNDEF!",
    [STE_VAR] = "var",
    [STE_STRUNION_DEF] = "s_u",
    [STE_FN] = "fnc"
};

static const char* linkage_str[] = {
    [L_UNDEF] = "UNDEF!",
    [L_NONE] = "NONE",
    [L_INTERNAL] = "INTERNAL",
    [L_EXTERNAL] = "EXTERNAL"
};


/*
 *  Dump a single symbol table (the current_scope) with basic list
 */
void st_dump_single() {
    st_dump_single_given(current_scope);
}

/*
 *  Dump a single st_entry with one-line info.
 */
void st_dump_entry(const st_entry* e) {
    if (!e) return;
    fprintf(stderr, "%s<%s> <ns:%s> <stor:%s%s> <scope:%s @ %s:%d>",
            st_entry_types_str[e->entry_type],
            e->ident,
            namespaces_str[e->ns],
            storspec_str[e->storspec],
            e->is_param ? "*" : "",
            scope_types_str[e->scope->scope_type],
            e->scope->context.filename,
            e->scope->context.lineno
    );
    if (e->scope == &root_symtab && e->linkage && e->linkage != L_NONE) {
        fprintf(stderr, " <linkage:%s>", linkage_str[e->linkage]);
    }
    if (e->decl_context.filename) {
        fprintf(stderr, " <decl %s:%d>", e->decl_context.filename, e->decl_context.lineno);
    }
    if ((e->ns == NS_TAGS && !e->members) || (e->entry_type == STE_FN && !e->fn_defined)) {
        fprintf(stderr, " (FWD-DECL)");
    }
    if (e->def_context.filename) {
        fprintf(stderr, " <def %s:%d>", e->def_context.filename, e->def_context.lineno);
    }
    fprintf(stderr, "STACK: %d", e->stack_offset);
    fprintf(stderr, "\n");
}

/*
 *  Output in-depth info for given st_entry based on entry_type.
 *  Call st_dump_entry() first if you want to get the symbol info line.
 */
void st_examine_given(const st_entry* e) {
    if (e->entry_type == STE_FN) {
        fprintf(stderr, "FUNCTION RETURNING:\n");
        print_ast(e->type);
        if (e->param_list) {
            fprintf(stderr, "AND TAKING PARAMS:\n");
            print_ast(e->param_list);
        } else {
            fprintf(stderr, "TAKING UNKNOWN/NO PARAMETERS\n");
        }
        if (e->fn_defined) {
            fprintf(stderr, "DUMPING FUNCTION SYMTAB:\n");
            st_dump_single_given(e->fn_scope);
            fprintf(stderr, "DUMPING FUNCTION AST:\n");
            print_ast(e->body);
        }
    }
    if (e->entry_type != STE_FN && (e->ns == NS_MEMBERS || e->ns == NS_MISC)) {
        print_ast(e->type);
    }
    if (e->entry_type == STE_STRUNION_DEF && e->members) {
        fprintf(stderr, "Dumping tag-type mini scope of tag <%s>!\n", e->ident);
        st_entry* cur = e->members->first;
        while (cur) {
            st_dump_entry(cur);
            st_examine_given(cur);
            cur = cur->next;
        }
    }
}

/*
 *  Search for an ident across all the namespaces and output st_entry info.
 */
void st_examine(const char* ident) {
    st_entry *e;
    int count = 0;       // yeah, weird way of getting the enum size
    for (unsigned i=0; i<sizeof(namespaces_str)/sizeof(namespaces_str[0]); i++) {
        if ((e = st_lookup(ident, i))) {
            count++;
            printf("> found <%s>, here is the entry (and type if applicable):\n", ident);
            st_dump_entry(e);
            st_examine_given(e);
        }
    }
    if (!count) {
        printf("> No matches found for <%s> using lookup from current scope (%p)\n", ident, (void*)current_scope);
    }
    printf("\n");
}


/*
 *  Search for a member of a tag and output st_entry info.
 */
void st_examine_member(const char* tag, const char* child) {
    st_entry *e = st_lookup(tag, NS_TAGS);
    if (!e) {
        printf("> I was not able to find tag <%s>.\n\n", tag);
        return;
    }
    if (!e->members) {
        printf("> Tag <%s> is declared, but not defined:\n", tag);
        st_dump_entry(e);
        printf("\n");
        return;
    }
    st_entry *m = st_lookup_fq(child, e->members, NS_MEMBERS);
    if (!m) {
        printf("> I found tag <%s>, but not the member <%s>:\n", tag, child);
        st_dump_entry(e);
        printf("\n");
        return;
    }
    printf("> Found <%s> -> <%s>:\n", tag, child);
    st_dump_entry(m);
    print_ast(m->type);
    return;
}

// kind of fake at the moment
void st_dump_recursive() {
    fprintf(stderr, "_- symtab dump for translation unit: -_\n");
    st_entry *e = current_scope->first;
    while (e) {
        st_dump_entry(e);
        st_examine_given(e);
        fprintf(stderr, "\n");
        e = e->next;
    }
}

static void st_dump_single_given(const symtab* s) {
    //printf("Dumping symbol table!\n");
    st_entry* cur = s->first;
    while (cur) {
        st_dump_entry(cur);
        cur = cur->next;
    }
}
