#ifndef IR_ARITHMETIC_H

#include "ast.h"
#include "ir.h"

bool type_is_arithmetic(const_astn a);
astn gen_add_rvalue(astn a, astn target);
astn gen_sub_rvalue(astn a, astn target);

#endif
