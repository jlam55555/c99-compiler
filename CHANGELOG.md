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
    - implemented getopt flags to read in from non-stdin, print out to
        non-stdout, print debug to non-stderr
    - implemented function return type is as declared, unless implicit (implicit
        return type is still int) or void (also defaults to int just in case
        it is used in intermediate values)
    - fixed implicit upcasting of longs to quadwords
    - added sample programs to res/ttests
    - removed branches to immediate next block (basic block fallthrough
        optimization)
    - updated README with new run instructions
