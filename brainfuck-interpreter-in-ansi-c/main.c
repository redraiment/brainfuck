#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define TOKENS "><+-.,[]"
#define CODE_SEGMENT_SIZE 30000
#define STACK_SEGMENT_SIZE 1000
#define DATA_SEGMENT_SIZE 30000

typedef void (*Instruction)(void);

struct {
  char cs[CODE_SEGMENT_SIZE];   /* Code Segment */
  long ip;                      /* Instruction Pointer */
  long ss[STACK_SEGMENT_SIZE];  /* Stack Segment */
  long sp;                      /* Stack Pointer */
  long ds[DATA_SEGMENT_SIZE];   /* Data Segment */
  long bp;                      /* Base Pointer */
  Instruction fn[128];          /* Instructions */
} vm;

void vm_forward() {
  vm.bp = (vm.bp + 1) % DATA_SEGMENT_SIZE;
}

void vm_backward() {
  vm.bp = (vm.bp + DATA_SEGMENT_SIZE - 1) % DATA_SEGMENT_SIZE;
}

void vm_increment() {
  vm.ds[vm.bp]++;
}

void vm_decrement() {
  vm.ds[vm.bp]--;
}

void vm_input() {
  vm.ds[vm.bp] = getchar();
}

void vm_output() {
  putchar(vm.ds[vm.bp]);
}

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

void vm_while_exit() {
  if (vm.sp > 0) {
    vm.sp--;
    if (vm.ds[vm.bp]) {
      vm.ip = vm.ss[vm.sp];
    }
  }
}

/**
 * 初始化虚拟机。
 * 1. 清空寄存器。
 * 2. 初始化指令。
 */
void initialize() {
  memset(&vm, 0, sizeof(vm));
  vm.fn['>'] = vm_forward;
  vm.fn['<'] = vm_backward;
  vm.fn['+'] = vm_increment;
  vm.fn['-'] = vm_decrement;
  vm.fn['.'] = vm_output;
  vm.fn[','] = vm_input;
  vm.fn['['] = vm_while_entry;
  vm.fn[']'] = vm_while_exit;
}

/**
 * 加载源码。
 */
void load() {
  int token;
  for (int index = 0; (token = getchar()) != EOF;) {
    if (strchr(TOKENS, token)) {
      vm.cs[index] = token;
      index++;
    }
  }
}

/**
 * 执行指令。
 */
void execute() {
  while (vm.cs[vm.ip]) {
    vm.fn[vm.cs[vm.ip]]();
    vm.ip++;
  }
}

int main(int argc, char* argv[]) {
  if (argc > 1) {
    freopen(argv[1], "r", stdin);
  }
  initialize();
  load();
  execute();
  return 0;
}
