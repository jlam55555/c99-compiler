cmake_minimum_required(VERSION 3.13)
project(ece466_compiler C)
set(CMAKE_C_STANDARD 11)

# bison/flex
find_package(BISON)
find_package(FLEX)
BISON_TARGET(parser ${CMAKE_SOURCE_DIR}/src/parser/parser.y
        ${CMAKE_BINARY_DIR}/parser.tab.c VERBOSE
        COMPILE_FLAGS --defines=${CMAKE_BINARY_DIR}/parser.tab.h)
FLEX_TARGET(lexer ${CMAKE_SOURCE_DIR}/src/lexer/lexer.l
        ${CMAKE_BINARY_DIR}/lex.yy.c
        COMPILE_FLAGS --header-file=${CMAKE_BINARY_DIR}/lex.yy.h)

# C source files and headers
FILE(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/src/*.c)
include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_BINARY_DIR})

add_executable(compiler
        ${BISON_parser_OUTPUTS}
        ${FLEX_lexer_OUTPUTS}
        ${SOURCES})
