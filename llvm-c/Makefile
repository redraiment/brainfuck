PROGRAM := compiler
OBJECTS := main.o options.o engine.o compiler.o

CC := clang
CFLAGS := $(shell llvm-config --cflags)
LDFLAGS := $(shell llvm-config --ldflags --libs)

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $^

clean:
	rm -f $(PROGRAM) $(OBJECTS)

.PHONY: clean all
