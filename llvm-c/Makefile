PROGRAM := brainfuck
OBJECTS := main.o options.o engine.o compiler.o linker.o

CC := clang
LEX := flex
CFLAGS := $(shell llvm-config --cflags)
LDFLAGS := $(shell llvm-config --ldflags --libs)

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $^

%.c: %.l
	$(LEX) -o $@ $^

clean:
	rm -f $(PROGRAM) $(OBJECTS)

.PHONY: clean all
