#ifndef IR_PRINT_H
#define IR_PRINT_H

#include "ast.h"
#include "ir.h"

#include <stdio.h>

const char *qoneword(const_astn a);
void quad_print(quad first);
void quads_dump_llvm(FILE *o);

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
