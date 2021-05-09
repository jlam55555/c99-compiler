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
- 4/26/21: continuing expression/control flow quad generation
    - refactored quad gen for expressions, control flow to new files
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
- 4/29/21: continuing expression/control flow quad generation
    - refactored gen_rvalue() to work for setting condition codes
    - refactored working bb into global cur_bb, which makes recursive bb control
        flow generation much easier (and more like Hak's notes)
    - introduced enum cc with condition codes (for results of relational
        operators, and for conditional jumps), and OC_SETcc opcodes
    - fixed if statements quad generation
    - implemented condition inversion in generate_conditional_quads()
    - implemented reversal of basic block quads once link_bb() is called on a
        bb (calling this indicates that a bb is complete)
    - implemented an explicit linearization of basic blocks using bb_ll_push(),
        which may be called when calling basic_block_new() with add_to_ll set,
        or by manually calling bb_ll_push() later (which may be desirable in
        control flow statements); this linearization order is stored in the
        basic block linked list bb_ll
    - implemented break/continue statements in loops
    - improved visual printing of basic blocks and quads using a {block} format
    - updated print_basic_blocks() to print in the explicit linearization order
        dictated by bb_ll
- 4/30/21: finishing up major parts of quad generation
    - indicate whether symbol is global, local, parameter, or implicit (will
        influence target addressing modes); requires associating scope
        information with each symbol as they are being inserted into the symbol
        table b/c scope is popped before quad generation
    - implemented do-while and for loops
    - implemented logical AND (&&) with implicit control flow
    - implemented relational (comparison) operators; depending on the target,
        either emit a SETCC opcode (if set to variable/used as intermediate
        value) or do not (condition code will be used by branching statement)
    - implemented quad generation for character and string literal constants,
        including a new addr type for strings (AT_STRING)
    - emit fatal warning on typedef (rather than SEGFAULT)
    - cleanup of many to-do items, documentation of broken/missing items
- 5/1/21: beginning target code generation
    - fixed Makefile so that build is actually a .PHONY target now
    - created a host of data structures for representing components of the
        x86_64 language
    - implemented some translation of quads to these new data structures
    - implemented some printing functions for asm components in AT&T syntax
    - implemented linked list of global variables for target directives
    - implemented function prologue/epilogue
- 5/8/21: target code generation
    - implemented register selection for all opcodes
    - implemented memory-backed register management
    - implemented x86_64 fncall api
    - implemented string constants as array-like types in the .rodata section
    - implemented rip-relative addressing for extern/static variables and
        string constants, and rbp-relative addressing for all local values
    - implemented directives for extern and static variables
    - fixed basic block ordering in for loops
- 5/9/21: touchups
    - implemented getopt flags to read in fron non-stdin, print out to
        non-stdout, print debug to non-stderr
    - added sample programs to res/ttests

TODO: remove branches to immediate next block
TODO: fix extra intermediate value for fncalls -- default rettype is int
    (4 bytes), so if you want to set something else you have to explicitly
    assign it directly to the type you want

---
  
[cmake-oos]: https://www.cs.swarthmore.edu/~adanner/tips/cmake.php
[styleguide]: https://www.kernel.org/doc/html/latest/process/coding-style.html