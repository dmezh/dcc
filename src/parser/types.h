#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>

#include "ast.h"
#include "types_common.h"

enum storspec describe_type(astn spec, struct astn_type *t);
void strict_qualify_type(astn qual, struct astn_type *t);

int get_sizeof(astn type);
astn descend_array(astn type);

#endif
