#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>

#include "ast.h"
#include "types_common.h"

enum storspec describe_type(struct astn *spec, struct astn_type *t);
void strict_qualify_type(struct astn *qual, struct astn_type *t);

#endif
