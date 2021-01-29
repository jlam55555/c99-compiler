lexer: lex.yy.c
	gcc -o lexer lex.yy.c

lex.yy.c: lexer.l
	flex lexer.l

clean:
	rm -f lex.yy.c lexer