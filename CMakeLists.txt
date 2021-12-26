cmake_minimum_required(VERSION 3.10)

project(DCC)

set(CMAKE_C_COMPILER clang)
add_compile_options(-Wall -Wextra -fsanitize=undefined -Wpedantic -Wunused -O2)
set(CMAKE_EXE_LINKER_FLAGS "-fsanitize=undefined -rdynamic")

include_directories(.)
include_directories(lexer)
include_directories(optimize)
include_directories(parser)
include_directories(quads)
include_directories(target_x86)
include_directories(common)

##################################################
# Make lexer and parser
##################################################

find_package(BISON)
find_package(FLEX)

set(GENERATED ${CMAKE_BINARY_DIR}/gen)
file(MAKE_DIRECTORY ${GENERATED})

BISON_TARGET(dcc_parser parser/parser.y
            ${GENERATED}/parser.tab.c
            DEFINES_FILE ${GENERATED}/parser.tab.h)

FLEX_TARGET(dcc_lexer lexer/lexer.l
            ${GENERATED}/lex.yy.c)

ADD_FLEX_BISON_DEPENDENCY(dcc_lexer dcc_parser)

include_directories(${GENERATED})

##################################################
##################################################

add_executable(dcc
${BISON_dcc_parser_OUTPUTS}
${FLEX_dcc_lexer_OUTPUTS}
    optimize/opt_flatten_adjacent_mov.c
    optimize/optimization.c
    parser/ast.c
    parser/ast_print.c
    parser/symtab.c
    parser/symtab_print.c
    parser/types.c
    quads/quads.c
    quads/quads_cf.c
    quads/quads_print.c
    target_x86/asmgen.c
    common/charutil
    common/semval.c
    common/util.c
    common/yak.ascii.c
    main.c
)

set_target_properties(dcc PROPERTIES C_STANDARD 11)
