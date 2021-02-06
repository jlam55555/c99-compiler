parser: parser.tab.o lex.yy.o numutils.o stringutils.o errorutils.o unicodeutils.o
	gcc -o parser parser.tab.o lex.yy.o numutils.o stringutils.o errorutils.o unicodeutils.o

parser.tab.o: parser.y
	bison -vd parser.y
	gcc -c -o parser.tab.o parser.tab.c

# lexer: lex.yy.o numutils.o stringutils.o errorutils.o unicodeutils.o
# 	gcc -o lexer lex.yy.o numutils.o stringutils.o errorutils.o unicodeutils.o

lex.yy.o: lex.yy.c
	gcc -c -o lex.yy.o lex.yy.c

lex.yy.c: lexer.l
	flex lexer.l

numutils.o: numutils.c
	gcc -c -o numutils.o numutils.c

stringutils.o: stringutils.c
	gcc -c -o stringutils.o stringutils.c

errorutils.o: errorutils.c
	gcc -c -o errorutils.o errorutils.c

unicodeutils.o: unicodeutils.c
	gcc -c -o unicodeutils.o unicodeutils.c

clean:
	rm -f lex.yy.c parser.tab.c *.o parser