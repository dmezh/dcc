cmake_minimum_required(VERSION 3.10)

project(DCC)

include_directories(1-lexer)
include_directories(2-parser)
include_directories(3-quads)
include_directories(target_x86)
include_directories(.)

##################################################
# Make lexer and parser
##################################################

find_package(BISON)
find_package(FLEX)

set(GENERATED ${CMAKE_CURRENT_BINARY_DIR}/gen)
file(MAKE_DIRECTORY ${GENERATED})

BISON_TARGET(dcc_parser 2-parser/parser.y
${GENERATED}/parser.tab.c
DEFINES_FILE ${GENERATED}/parser.tab.h)

FLEX_TARGET(dcc_lexer 1-lexer/lexer.l
${GENERATED}/lex.yy.c)

ADD_FLEX_BISON_DEPENDENCY(dcc_lexer dcc_parser)

include_directories(${GENERATED})

##################################################
##################################################

add_executable(dcc
${BISON_dcc_parser_OUTPUTS}
${FLEX_dcc_lexer_OUTPUTS}
2-parser/ast.c
2-parser/symtab.c
2-parser/types.c
3-quads/quads.c
3-quads/quads_cf.c
3-quads/quads_print.c
target_x86/asmgen.c
charutil.c
semval.c
util.c
yak.ascii.c
)

set_target_properties(dcc PROPERTIES
RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/2-parser"
C_STANDARD 11
)