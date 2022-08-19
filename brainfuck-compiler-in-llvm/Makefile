PROGRAM := brainfuck
OBJECTS := main.o llvm.o

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
