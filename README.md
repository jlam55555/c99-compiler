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

If using an IDE with cmake plugin support (e.g., cmake-tools in vscode), set
the build directory to the `target` directory.

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

##### Declarations and Scopes
The declaration parser handles the syntax from 6.5. (Abstract declarators
and typenames are in 6.6 and are not currently implemented, but they should
be for the next checkpoint.) This includes most regular declarations (variables
and functions), struct/union tag declarations and definitions, arbitrarily
complex declarators, and a basic symbol table and scope implementation. Scopes
can be arbitrarily nested.

Not implemented:
- enums
- bitfields
- typedefs
- initializers
- old function definition syntax
- most functionality related to type qualifiers and storage class specifiers

##### Statements
The top-level non-terminal, the translation unit, is finally here! Function
definitions and prototype scopes are implemented. A universal AST node print
function can print the entire AST statement tree (declarations are printed
separately, as they are being declared). Statements (expression statements,
block statements, and control flow), labels have been implemented.

TODO
- Redeclaration of extern variables is allowed, but need to check for
    compatibility and narrow type to strictest intersection of the two types
    (for now redeclaration of extern variable with any type is allowed (BAD))
- Recursive declaration validation
- Convert array to pointers in function parameter lists

##### Quad Generation
TODO
- Everything

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
- 3/18/21: new datatypes for TLD
    - new AST types for "everything else" outside of expressions and
        declarations: blocks, statements, function defs, translation unit
        (top-level)
    - updated parser with remaining syntax rules
- 3/20/21: bug fixes and scopes
    - fixed new shift/reduce conflicts
    - fixed lineno off-by-one-error and removed bison %locations
    - fixed struct not printing if predeclared
    - fixed struct printing line where declared, not line where defined
    - added scoping (compound statements trigger pushing and popping of scopes)
- 4/3/21: finishing assignment 3, doing assignment 4
    - implemented abstract declarators and typenames, and the casting and sizeof
        (with typenames) operators
    - moved C code away from parser.y into main.c
    - fixed struct declarations that unintentionally behaved like forward
        declarations
    - implemented prototype scopes, which automatically promote to function
        scopes in a function definition
    - implemented a printing function for statements/blocks
    - identifiers in expressions now point to their symbol table entries, and
        throw an error if undeclared
    - implemented labels
- 4/9/21: finishing assignments 3/4, beginning assignment 5
    - allow void as (only) parameter in param list for function definition
    - allow redeclaration of extern variables (need to check if types are
        compatible and create the strict intersection of the types)
    - fix unions being printed as structs
    - fix unnamed unions not being assigned a filename/lineno location
    - implementing labels

---
  
[cmake-oos]: https://www.cs.swarthmore.edu/~adanner/tips/cmake.php