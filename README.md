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
The declaration parser handles the syntax from 6.5. (Abstract declarators
and typenames are in 6.6 and are not currently implemented, but they should
be for the next checkpoint.) This includes most regular declarations (variables
and functions), struct/union tag declarations and definitions, arbitrarily
complex declarators, and a basic symbol table and scope implementation.
(In the absence of function definitions and blocks, the only scopes are
currently the global scope and struct/union "scopes.")

Not implemented:
- enums
- bitfields
- typedefs
- initializers
- most functionality related to type qualifiers and storage class specifiers

TODO (for next checkpoint):
- Check that declaration specifier combination is valid (recursively)
    before inserting symbol (a lot of work to be done here)
- Move extra printing in parser.y to printutils.c
- Abstract declarators and typenames (should be a relatively trivial extension
    to existing (regular) declarators)
- Scoping (function, block, prototype)

---

### Changelog
 - 3/17/21: start of changelog, submission of assignment 3 (declarations)
    - refactored a large part of the codebase to clean up code: split asttypes.c
        into smaller files and functions with better commenting; stopped abusing
        macros for things macros should not be used for; wrote macros for
        common linked-list operations
    - rewrote data structures for complex declarations; now pointers, arrays,
        functions, and declaration specifiers all somewhat-equal, which makes
        them much easier to link together and interpret
    - changed build system from custom Makefile to cmake, as well as separating
        src/ and include/ in the directory structure due to the large number
        of total source files
    - updated README

---
  
[cmake-oos]: https://www.cs.swarthmore.edu/~adanner/tips/cmake.php