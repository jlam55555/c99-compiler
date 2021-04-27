# ece466-compiler
A basic C compiler for ECE466

---

### Build instructions
Dependencies:
- `cmake` (3.13+)
- `make`
- `flex`
- `bison`
- `gcc` (tested on gcc8)

The top-level Makefile in this directory has some recipes for building
[out-of-source][cmake-oos]:
```bash
$ make build run
$ make clean
```
This will build to the `build` directory.

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

##### Quad Generation
Quad generation is partitioned into expression quad generation and statement
(control flow) quad generation. There are three new struct types: basic_block,
quad, and addr (representing an operand to/destination of a quad).

Each quad is generic (e.g., operand types and sizes may vary, e.g., the MOV
quad encompasses MOVB, MOVL, etc. and may have any combination of operand
addressing modes, even if illegal in x86_64). Each struct addr is given a type
using the same typing system as the AST nodes, and thus each (sub)expression
can be type-checked. With the more verbose DEBUG2 debug level, all of the
struct addr types are printed for your viewing pleasure. The sizeof operator
is necessarily implemented as a compile-time construct.

TODO: write about statement/basic block generation

Not implemented:
- non-int/char lvalues (yet?)
- structs/unions lvalues (yet?)
- ternary statements
- goto and switch statements

TODO (also see res/scratch/TODO)
- Statements
    - compound statements -- done?
    - expression statements -- work in progress
    - if statements
    - loop statements
        - break/continue statements
    - return statements
- Indicate whether variables are local (parameter or regular) or global
    (for use with addressing modes later)

Notes:
- At this point, we start making assumptions about the architecture. In
    particular, a 64-bit architecture based on the common x86_64 model is used.

Fixes (from previous assignment):
- Labels should be inserted into the symbol table, and unresolved goto labels
    should be resolved when function is complete
- Check that member access on a struct/union is a valid member name
- Don't segfault on seeing typedef
- Allow arrays (including VLAs) in prototypes
- Redeclaration of extern variables is allowed, but need to check for
    compatibility and narrow type to strictest intersection of the two types
    (for now redeclaration of extern variable with any type is allowed (BAD))
- Recursive declaration validation
- Convert array to pointers in function parameter lists

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
    - introduced preliminary data structures for quads (quads, basic blocks)
- 4/12/21: beginning quad generation
    - introduced addr object type for quad operands and output; can hold
        constant values, temporary values (i.e., a pseudo-register), and symbols
    - implemented sizeof operator for some symbols
    - implemented quad generation for labels and simple arithmetic expressions
    - implemented printing for current basic block structures
- 4/17/21: continuing quad generation
    - updated numeric literals (in lexer) to use the parser representation, to
        avoid having multiple type representations
    - updated struct addr to hold a type representation
    - improved 3-addr quad generation to remove extra MOV operations (for
        slightly more compact ("optimized"?) quads, but these will be turned
        back into MOV operations because x86 is a 2-address architecture)
    - implemented more arithmetic operations
- 4/24/21: continuing quad generation
    - updated (most) quad expression function names to match Hak's notes
    - added more functionality to gen_rvalue function
    - implemented gen_lvalue, gen_assign functions
    - implemented most array/pointer dereferencing behavior, including:
        - pointer addition (not subtraction yet)
        - multidimensional arrays
        - dereferencing of pointer/array to array (no-op/reinterpret cast-like)
- 4/26/21: continuing expression quad generation
    - refactored quad gen for expressions to new file
    - updated sizeof(expr) to output quads to dummy bb so they don't show up
        in the final quad output
    - consolidated all dereferencing, variable values from gen_rvalue into
        gen_lvalue to avoid duplication
    - implemented arrays demote to regular pointers for all operations except
        direct argument to sizeof
    - implemented a second debugging level (DEBUG2) for more debug info than
        what Hak puts in his assignments
        - DEBUG2 prints out type information for struct addr
        - DEBUG2 makes sizeof(expr) emit quads to regular (non-dummy) output
    - implemented pointer subtraction
    - implemented addressof (&) operator (noop/reinterpret cast for arrays)
    - implemented explicit type conversion
    - implemented function calls (and don't allow function calls on non-function
        objects)
    - TODO: set correct output type based on input types (implicit conversions)
    - TODO: implement all operations
    - TODO: warn if statement is useless (i.e., no side effects)
    - TODO: write a function to check if two types match (for extern decl
        compatibility check and for pointer assignment)
    - TODO: indicate if variables are local or global

---
  
[cmake-oos]: https://www.cs.swarthmore.edu/~adanner/tips/cmake.php