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

### Run Instructions
```bash
$ path/to/compiler -o [OUT_FILE] -d [DEBUG_OUT_FILE] [INFILE1] [INFILE2] ...
```
All of the options are optional: the compiler will use stdin for input, stdout
for asm output, and stderr for debug output by default. By default,
`path/to/compiler` will be `build/compiler` (built by cmake). The input files
should be preprocessed (`gcc -E`).

Sample usage: There are some sample programs in `res/ttests` that should
compile correctly.
```bash
$ gcc -E res/ttests/*.c | build/compiler -o testcases.S -d debug.txt \
    && gcc -m64 -o testcases testcases.S && ./testcases
```
(Compare the output to: ```gcc res/ttests/*.c && ./a.out`.)

---

### Code Style
Code style roughly follows the [Linux kernel coding style][styleguide], which
itself has an affinity for K&R conventions.

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
quad, and addr (representing an operand to/destination of a quad). There are two
new enums: opcode (for quad opcodes), and cc (for condition codes, used for
relational operators and branching statements).

Each quad is generic (e.g., operand types and sizes may vary, e.g., the MOV
quad encompasses MOVB, MOVL, etc. and may have any combination of operand
addressing modes, even if illegal in x86_64). Each struct addr is given a type
using the same typing system as the AST nodes, and thus each (sub)expression
can be type-checked. With the more verbose DEBUG2 debug level, all of the
struct addr types are printed for your viewing pleasure. The sizeof operator
is necessarily implemented as a compile-time construct.

The control flow graph (CFG) is developed as a graph of struct basic_block
objects, and the linearization is controlled by the linked list ll_bb. Control
flow for if/else, while, do-while, and for statements are implemented. If/else
statements use condition inversion and while loops have the condition after
the body to reduce the number of branches (i.e., increase fall-throughs). The
logical AND (&&) operator was implemented with implicit control flow.

Notes:
- At this point, we start making assumptions about the architecture. In
    particular, a 64-bit architecture based on the common x86_64 model is used.
- Explicit casting is supported but (very) weakly checked. See the note about
    OC_CAST.
- Each struct addr has a (single) type declaration associated with it
    (addr->decl), which allows for type-checking. However, since typing is
    strict, casting (which may be a noop) has to be represented manually, so we
    introduce the OC_CAST quad. E.g., this happens when taking the address of
    an array (a noop, but the type changes from array[] to pointer->array[]).
    The resulting target code generated from the OC_CAST will depend on the
    types of its operands. (e.g., the above will generate a noop but a signed
    cast from long to quadword will result in a CLTQ opcode.)
- In general, there is the capability for type checking (via the typed struct
    addr) but we don't have the time to check that every operation is valid.
    It is left to the programmer to explicitly cast types when necessary, as
    there may be unfortunately arbitrary implicit casts when determining the
    type of new temporary struct addr objects. The safest bet is to just use
    a single size and signed-ness for any operation.

Not (fully?) implemented:
- non-int/char lvalues (yet?)
- structs/unions lvalues (yet?) and thus member operations (. and ->)
- ternary statements
- goto and switch statements
- warn if statement is useless
- logical OR (||) -- mostly because lazy, logic is same as &&
- bitwise operators and postinc/postdec (same reason: not hard, just tedious)
- a lot of type checking and integer promotion -- for now, assume arithmetic
    operations occur on integral items of the same type, all casts are valid, 
    not assigning to arrays or function lvalues
- sizeof struct may be incorrect: for now, simply sums the sizes of its
    component members (since we're not really implementing struct lvalues at
    all, this is a lesser worry)
- function prototypes are allowed in the syntax, but they (currently) are not
    at all type-checked during quad generation; it is left to the programmer
    to make sure the types are correct
- functions have an implicit return 0; appended to their function body if the
    last statement is not a return statement

Unresolved (from previous assignment):
- Labels should be inserted into the symbol table, and unresolved goto labels
    should be resolved when function is complete
- Check that member access on a struct/union is a valid member name
- Allow arrays (including VLAs) in prototypes
- Redeclaration of extern variables is allowed, but need to check for
    compatibility and narrow type to strictest intersection of the two types
    (for now redeclaration of extern variable with any type is allowed (BAD))
- Recursive declaration validation
- Convert array to pointers in function parameter lists

##### Target Code Generation
Working x86_64 GAS assembly code is produced that is mildly compliant with the
function calling API. A very simple (one-to-one) instruction selection scheme
was used to translate quads into opcodes of the appropriate size. All temporary
values are memory-backed on the stack like local variables for simplicity.
Extern and static variables are declared by the appopriate directives and
referenced using RIP-relative addressing to allow for PIC executables. Object
files generated by this compiler successfully link with gcc-compiled objects.

Not implemented:
- calling a function value that is not directly a declarator (e.g., (*g)())
- function calls or function definitions with more than 6 parameters
- proper/best alignment for function stacks and local variables
- a decent register allocation scheme
- as before, support for structs and fp values is omitted

Semantic notes:
- String literals are implemented like character arrays (in .rodata), since they
    are memory addresses; their array length is set to 8 so that sizeof works
    correctly
- Currently, the program allows you to input multiple source files but outputs
    the entire output into one output file. This is usually fine but may cause
    problems (e.g., multiple static variables with the same name). If necessary,
    compile files separately.

---

### Changelog
See [CHANGELOG.md](./CHANGELOG.md).

[cmake-oos]: https://www.cs.swarthmore.edu/~adanner/tips/cmake.php
[styleguide]: https://www.kernel.org/doc/html/latest/process/coding-style.html
