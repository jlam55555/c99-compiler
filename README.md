# ece466-compiler
A basic C compiler for ECE466

### Lexer
Most of the C11 lexical rules were implemented using Flex.

This lexer supports valid UTF-8 source code; i.e., strings and character
constants can contain UTF-8. Invalid UTF-8 will be interpreted as
octal constants.

##### Tests
This will run the lexer on the test cases. The diff from the
expected output will be found in [diffs/diff.out](diffs/diff.out).
```bash
$ gcc -E ltests/*.c|./lexer >diffs/test.out 2>diffs/test.err
$ diff diffs/test.out ltests/ltest.out >diffs/diff.out
```
(No test cases are provided for wide characters or UTF-8.)