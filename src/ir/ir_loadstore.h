#ifndef IR_LOADSTORE_H
#define IR_LOADSTORE_H

#include "ast.h"

astn gen_load(astn a, astn target);
astn gen_store(astn target, astn val);
astn gen_assign(astn a);
astn gen_select(astn a);

#endif
