# This Makefile builds the parser
# TODO: use src/ include/ build/ bin/ directory structure?
SOURCEDIR=. lexerutils
BUILDDIR=build
BINARY=parser

# C files that are built by flex/bison and cannot be found automatically
EXTRASOURCES=lex.yy.c parser.tab.c

# extra headers (to be removed on make clean)
EXTRAHEADERS=lex.yy.h parser.tab.h

# "build everything" inspired by https://stackoverflow.com/a/3774731
SOURCES:=$(shell find $(SOURCEDIR) -maxdepth 1 -name '*.c') $(EXTRASOURCES)
OBJECTS:=$(addprefix $(BUILDDIR)/,$(SOURCES:%.c=%.o))

$(BINARY): $(EXTRAHEADERS) $(OBJECTS)
	$(CC) -o $(BINARY) $(OBJECTS)

# special targets (flex/bison)
lex.yy.c lex.yy.h: lexer.l parser.tab.h
	flex --header-file=lex.yy.h lexer.l

parser.tab.c parser.tab.h: parser.y
	bison -vd parser.y

# generic targets
$(BUILDDIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) -c -o $@ $<

.PHONY:
clean:
	rm -rf build $(EXTRASOURCES) $(EXTRAHEADERS) $(BINARY)