# ece466-compiler
A basic C compiler for ECE466

### Lexer (Assignment 1)
Most of the C11 lexical rules were implemented using Flex.

This lexer supports valid UTF-8 source code; i.e., strings and character
constants can contain UTF-8. Invalid UTF-8 will be interpreted as
octal constants.

##### Build instructions
```bash
$ make lexertest
```

##### Test cases
```bash
$ gcc -E ltests/*.c|./lexertest 2>diffs/test.err | diff - ltests/ltest.out
```

### Expression Parser (Assignment 2)
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

##### Build instructions
```bash
$ make parser
```

##### Test cases
```bash
$ ./parser <ptests/exprtests.c | diff - ptests/exprtests.out
```
Note that some of the test cases were fixed to make them more consistent.
The output for numeric and character constants is also a little different
but the content is the same.

### Declaration Parser (Assignment 3)
TODO:
- Finish basic declaration handling
- Finish basic insertion into the symbol table
- Finish this writeup about the declaration parser
- Implement error handling/warning
- Typedef handling (kludges!)
- Cleanup and better documentation: comments at the
  beginning of each file describing them