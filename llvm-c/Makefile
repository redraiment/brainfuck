PROGRAM := brainfuck
SOURCES := main.c options.c engine.c ast.c scanner.c parser.c compiler.c linker.c

CC := clang
LEX := flex
YACC := bison
CFLAGS := $(shell llvm-config --cflags)
LDFLAGS := $(shell llvm-config --ldflags --libs)

all: $(PROGRAM)

$(PROGRAM): $(SOURCES)
	$(CC) -lfl -ly $(CFLAGS) $(LDFLAGS) -o $@ $^

# %.o: %.c
# 	$(CC) -c $(CFLAGS) -o $@ $^

%.c: %.y
	$(YACC) -d -o $@ $^

%.c: %.l
	$(LEX) --header-file=$(^:.l=.h) -o $@ $^

clean:
	rm -f $(PROGRAM) $(OBJECTS)

.PHONY: clean all
