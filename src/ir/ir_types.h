#ifndef IR_TYPES_H
#define IR_TYPES_H

#include "ast.h"

bool is_integer(astn a);
ir_type_E ir_type(astn a);
bool ir_type_matches(astn a, ir_type_E t);
astn get_qtype(astn t);

#endif
