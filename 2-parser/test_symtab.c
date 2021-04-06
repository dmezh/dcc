#include <stdio.h>
#include "symtab.h"

int main() {
    st_insert("test1");
    st_insert("test2");
    st_insert("test3");
    st_insert("test4");
    st_insert("test5");
    st_dump_single();
    if (st_lookup("test1")) {
        printf("found test1!\n");
    }
    if (!st_lookup("test10")) {
        printf("good, I didn't find a fictional test10\n");
    }
    if (!st_insert("test1")) {
        printf("good, I was not allowed to insert test1, dumping again:\n");
        st_dump_single();
    }
    if (st_insert("test6")) {
        printf("good, I was allowed to insert test6, dumping again:\n");
        st_dump_single();
    }
    destroy_symtab(current_scope);
}
