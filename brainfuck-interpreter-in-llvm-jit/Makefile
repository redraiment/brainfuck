PROGRAM := brainfuck
OBJECTS := main.o

CC := clang
CFLAGS := $(shell llvm-config --cflags)
LDFLAGS := $(shell llvm-config --ldflags --libs)

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $^

clean:
	rm -f $(PROGRAM) $(OBJECTS) sum*

.PHONY: clean all
