# ece466-compiler
A basic C compiler for ECE466

---

### Build instructions
This requires `cmake` (version 3) and `make`. The top-level Makefile
in this directory has some recipes for building it
[out-of-source][cmake-oos]:
```bash
$ make build run
$ make clean
```

---

### Implementation details
This parser will attempt to implement much of the C99 standard, except
where specified. The lexer also includes some C11 support (namely,
unicode).

##### Lexing
Most of the C11 lexical rules were implemented using Flex.

This lexer supports valid UTF-8 source code; i.e., strings and character
constants can contain UTF-8. Invalid UTF-8 will be interpreted as
octal constants.

##### Expressions
This expression parser handles all of the C99 expression syntax
(6.5.1-6.5.4) excluding abstract typenames (compound literals and casting).
It builds an AST using the union recommendation with a generic "interface"
with different node types for number literals, string literals, character
constants, identifiers, binary operations, unary operations, the ternary
operator, and function calls.

The "interface" of the AST node includes a type descriptor and a "next"
pointer to another AST node, so that each node can act as a linked list
without a special linked-list type (e.g., for function argument lists or
initializer lists).

##### Declarations
TODO: Finish this writeup about the declaration parser
TODO: Fix error with short not being recognized
  
[cmake-oos]: https://www.cs.swarthmore.edu/~adanner/tips/cmake.php