#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define TOKENS "><+-.,[]"
#define CODE_SEGMENT_SIZE 30000
#define STACK_SEGMENT_SIZE 1000
#define DATA_SEGMENT_SIZE 30000

#define ERROR_CODE_MISSING_FILE 1
#define ERROR_CODE_READING_FILE 2

typedef void (*Instruction)(void);

/* Define a virutal machine */
struct {
  char cs[CODE_SEGMENT_SIZE];   /* Code Segment */
  long ip;                      /* Instruction Pointer */
  long ss[STACK_SEGMENT_SIZE];  /* Stack Segment */
  long sp;                      /* Stack Pointer */
  long ds[DATA_SEGMENT_SIZE];   /* Data Segment */
  long bp;                      /* Base Pointer */
  Instruction cmds[128];        /* Instructions */
} vm;

/* Command `>` */
void vm_forward() {
  vm.bp = (vm.bp + 1) % DATA_SEGMENT_SIZE;
}

/* Command `<` */
void vm_backward() {
  vm.bp = (vm.bp + DATA_SEGMENT_SIZE - 1) % DATA_SEGMENT_SIZE;
}

/* Command `+` */
void vm_increment() {
  vm.ds[vm.bp]++;
}

/* Command `-` */
void vm_decrement() {
  vm.ds[vm.bp]--;
}

/* Command `,` */
void vm_input() {
  char byte = getchar();
  vm.ds[vm.bp] = (byte != EOF) ? byte : 0; /* Set to zero if EOF */
}

/* Command `.` */
void vm_output() {
  putchar(vm.ds[vm.bp]);
}

/* Command `[` */
void vm_while_entry() {
  if (vm.ds[vm.bp]) {
    vm.ss[vm.sp] = vm.ip - 1;
    vm.sp++;
  } else {
    int level = 1;
    for (vm.ip++; vm.cs[vm.ip] && level; vm.ip++) {
      if (vm.cs[vm.ip] == '[') {
        level++;
      } else if (vm.cs[vm.ip] == ']') {
        level--;
      }
    }
    vm.ip--;
  }
}

/* Command `]` */
void vm_while_exit() {
  if (vm.sp > 0) {
    vm.sp--;
    if (vm.ds[vm.bp]) {
      vm.ip = vm.ss[vm.sp];
    }
  }
}

/**
 * Initialize the Virtual Machine.
 * 1. reset the memory to zero.
 * 2. setup eight commands.
 */
void initialize() {
  memset(&vm, 0, sizeof(vm));
  vm.cmds['>'] = vm_forward;
  vm.cmds['<'] = vm_backward;
  vm.cmds['+'] = vm_increment;
  vm.cmds['-'] = vm_decrement;
  vm.cmds['.'] = vm_output;
  vm.cmds[','] = vm_input;
  vm.cmds['['] = vm_while_entry;
  vm.cmds[']'] = vm_while_exit;
}

/**
 * Loads source codes from file to code segment.
 */
void load(char* filename) {
  int token;

  FILE* fin = fopen(filename, "r");
  if (fin == NULL) {
    fprintf(stderr, "read source code from %s failed!\n", filename);
    exit(ERROR_CODE_READING_FILE);
  }

  for (int index = 0; (token = fgetc(fin)) != EOF;) {
    if (strchr(TOKENS, token)) {
      vm.cs[index] = token;
      index++;
    }
  }
  fclose(fin);
}

/**
 * Executes commands one by one.
 */
void execute() {
  while (vm.cs[vm.ip]) {
    vm.cmds[vm.cs[vm.ip]]();
    vm.ip++;
  }
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <source-file>\n", argv[0]);
    return ERROR_CODE_MISSING_FILE;
  }

  initialize();
  load(argv[1]);
  execute();
  return EXIT_SUCCESS;
}
