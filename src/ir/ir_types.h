#ifndef IR_TYPES_H
#define IR_TYPES_H

#include "ast.h"

bool is_integer(astn a);
ir_type_E ir_type(astn a);
astn ir_dtype(astn t);
bool ir_type_matches(astn a, ir_type_E t);
astn get_qtype(astn t);

astn convert_integer_type(astn a, ir_type_E t);
astn do_arithmetic_conversions(astn a, astn b, astn *a_new, astn *b_new);
astn do_integer_conversions(astn a, astn b, astn *a_new, astn *b_new);

#endif
