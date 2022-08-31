#include "ir_initializers.h"

#include "ir_types.h"
#include "ir_util.h"

astn gen_initializer(astn a, astn g) {
    // cases:
    // a = ptr or arithmetic -> i = gen_rvalue(g)
    // a = static array, g = strlit -> i = strlit + zero
    // a = auto array, g = strlit -> make anonymous zstrlit, emit memcpy

    switch (a->type) {
        case ASTN_QTEMP:
            break;

        default:
            qunimpl(a, "Only qtemps are allowed as targets for gen_initializer.");
    }

    qunimpl(a, "Unreachable");
}
