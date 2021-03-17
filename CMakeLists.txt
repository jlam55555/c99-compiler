cmake_minimum_required(VERSION 3.14)
project(ece466_compiler C)

set(CMAKE_C_STANDARD 11)

include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_BINARY_DIR})

find_package(BISON)
BISON_TARGET(parser ${CMAKE_SOURCE_DIR}/src/parser.y
        ${CMAKE_BINARY_DIR}/parser.tab.c VERBOSE
        COMPILE_FLAGS --defines=${CMAKE_BINARY_DIR}/parser.tab.h)

find_package(FLEX)
FLEX_TARGET(lexer ${CMAKE_SOURCE_DIR}/src/lexer.l
        ${CMAKE_BINARY_DIR}/lex.yy.c
        COMPILE_FLAGS --header-file=${CMAKE_BINARY_DIR}/lex.yy.h)

# redundant but ok
ADD_FLEX_BISON_DEPENDENCY(lexer parser)

add_executable(compiler
        ${BISON_parser_OUTPUTS}
        ${FLEX_lexer_OUTPUTS}
        src/lexerutils/errorutils.c
        src/lexerutils/numutils.c
        src/lexerutils/stringutils.c
        src/lexerutils/unicodeutils.c
        src/common.c
        src/decl.c
        src/declspec.c
        src/printutils.c
        src/scope.c
        src/structunion.c
        src/symtab.c)
