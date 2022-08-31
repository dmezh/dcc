#ifndef IR_LVALUE_H
#define IR_LVALUE_H

#include "ast.h"

astn gen_indirection(astn a);
astn gen_lvalue(astn a);
astn lvalue_to_rvalue(astn a, astn target);

#endif
