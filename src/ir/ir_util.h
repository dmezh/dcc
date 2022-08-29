#ifndef IR_UTIL_H
#define IR_UTIL_H

#include "ast.h"
#include "ir.h"

#include <stdio.h>

BB bb_alloc(void);
BBL bbl_next(const BBL bbl);
BB bbl_this(const BBL bbl);
void bbl_append(BB bb);

astn new_qtemp(astn qtype);
astn qprepare_target(astn target, astn qtype);
quad last_in_bb(BB bb);

quad emit(ir_op_E op, astn target, astn src1, astn src2);
quad emit4(ir_op_E op, astn target, astn src1, astn src2, astn src4);

#define qwarn(...)  fprintf(stderr, "\n" __VA_ARGS__);
#define qunimpl(node, msg)  \
    {                       \
        qwarn("UH OH:\n");  \
        print_ast(node);    \
        die(msg);           \
    }

#define qerror(...)                                         \
    {                                                       \
        RED_ERROR("Error generating quads: " __VA_ARGS__);  \
    }

#define qprintcontext(context)                                                 \
    {                                                                          \
        fprintf(stderr, "** At <%s:%d>: ", context.filename, context.lineno);  \
    }

#endif
