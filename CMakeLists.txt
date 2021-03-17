cmake_minimum_required(VERSION 3.14)
project(ece466_compiler C)

set(CMAKE_C_STANDARD 11)

include_directories(include)

find_package(BISON)
BISON_TARGET(parser src/parser.y src/parser.tab.c VERBOSE COMPILE_FLAGS --defines=include/parser.tab.h)

find_package(FLEX)
FLEX_TARGET(lexer src/lexer.l src/lex.yy.c COMPILE_FLAGS --header-file=include/lex.yy.h)

# redundant but ok
ADD_FLEX_BISON_DEPENDENCY(lexer parser)

add_executable(compiler
        ${BISON_parser_OUTPUTS}
        ${FLEX_lexer_OUTPUTS}
        src/lexer.l
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
