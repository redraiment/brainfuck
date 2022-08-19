/**
 * Brainfuck language command builder.
 */
#include <stdlib.h>
#include <stdio.h>

#include "llvm.h"
#include "lang.h"

/**
 * Stack segment for loop.
 */
typedef struct _Stack {
  LLVMBasicBlockRef entry;
  LLVMBasicBlockRef body;
  struct _Stack* next;
} *Stack;
Stack stack = NULL;

/**
 * Obtain the top loop entry block.
 */
static LLVMBasicBlockRef CurrentEntryBlock() {
  return stack != NULL? stack->entry: NULL;
}

/**
 * Obtain the top loop body block.
 */
static LLVMBasicBlockRef CurrentBodyBlock() {
  return stack != NULL? stack->body: NULL;
}

/**
 * Push the block to top.
 */
static void push(LLVMBasicBlockRef entry, LLVMBasicBlockRef body) {
  Stack top = (Stack)calloc(sizeof(struct _Stack), 1);
  top->entry = entry;
  top->body = body;
  top->next = stack;
  stack = top;
}

/**
 * Pop the top block.
 */
static void pop() {
  if (stack != NULL) {
    Stack top = stack;
    stack = stack->next;
    free(top);
  }
}

/**
 * Remove all blocks.
 */
static void BrainfuckTearDown(void) {
  while (stack != NULL) {
    pop();
  }
}

/**
 * Imported external function symbols.
 */
typedef enum {
  s_getchar = 0,
  s_putchar,
  s_max,
  s_main,
  s_count
} Symbol;

/**
 * Static symbol table, fixed size.
 */
static struct {
  LLVMTypeRef types[s_count];
  LLVMValueRef values[s_count];
} SymbolTable;

/**
 * Import function.
 */
static LLVMValueRef declare(Symbol symbol, char* name, LLVMTypeRef type) {
  SymbolTable.types[symbol] = type;
  SymbolTable.values[symbol] = DeclareFunction(name, type);
  return SymbolTable.values[symbol];
}

/**
 * Call imported function.
 */
static LLVMValueRef invoke(Symbol symbol, int length, LLVMValueRef* parameters) {
  LLVMTypeRef type = SymbolTable.types[symbol];
  LLVMValueRef fn = SymbolTable.values[symbol];
  return call(type, fn, length, parameters);
}

/* Data Pointer */
static LLVMValueRef dp = NULL;

/**
 * Get value of the data pointer.
 */
static LLVMValueRef get() {
  LLVMValueRef pointer = load(LLVMPointerType(LLVMInt8Type(), 0), dp);
  return load(LLVMInt8Type(), pointer);
}

/**
 * Set value to the data pointer.
 */
static void set(LLVMValueRef value) {
  LLVMValueRef pointer = load(LLVMPointerType(LLVMInt8Type(), 0), dp);
  store(pointer, value);
}

/**
 * Create global data segment and return the data pointer.
 */
static LLVMValueRef CreateDataSegment() {
  LLVMTypeRef type = LLVMArrayType(LLVMInt8Type(), DATA_SEGMENT_SIZE);
  LLVMValueRef value = ConstZeroArray(LLVMInt8Type(), DATA_SEGMENT_SIZE);
  LLVMValueRef ds = DeclareGlobalVariableWithValue("ds", type, value);
  return Pointer(type, ds, 2, (LLVMValueRef[]){ Int32(0), Int32(0) });
}

/**
 * Create basic block and append to main function.
 */
static LLVMBasicBlockRef CreateBlock() {
  LLVMValueRef fn = SymbolTable.values[s_main];
  return Block(fn);
}

/**
 * Setup brainfuck skeleton.
 * - declare getchar.
 * - declare putchar.
 * - declare llvm.smax.i32.
 * - declare global data segment.
 * - create data pointer.
 */
void BrainfuckSetUp(void) {
  atexit(BrainfuckTearDown);

  LLVMValueRef ds = CreateDataSegment();

  declare(s_getchar, "getchar", LLVMFunctionType(LLVMInt32Type(), (LLVMTypeRef[]){}, 0, False));
  declare(s_putchar, "putchar", LLVMFunctionType(LLVMInt32Type(), (LLVMTypeRef[]){ LLVMInt32Type() }, 1, False));
  declare(s_max, "llvm.smax.i32", LLVMFunctionType(LLVMInt32Type(), (LLVMTypeRef[]){ LLVMInt32Type(), LLVMInt32Type() }, 2, False));
  declare(s_main, "main", LLVMFunctionType(LLVMInt32Type(), (LLVMTypeRef[]){}, 0, False));

  enter(CreateBlock());

  dp = alloc(LLVMPointerType(LLVMInt8Type(), 0));
  store(dp, ds);
}

/**
 * Build command `[`: while loop begin.
 */
void whileNotZero(void) {
  LLVMBasicBlockRef entry = CreateBlock();
  jumpTo(entry);

  LLVMBasicBlockRef body = CreateBlock();
  push(entry, body);
  enter(body);
}

/**
 * Build command `]`: while loop end.
 */
void whileEnd(void) {
  // body
  LLVMBasicBlockRef entry = CurrentEntryBlock();
  jumpTo(entry);

  // entry
  enter(entry);
  LLVMValueRef value = get();
  LLVMValueRef condition = compare(LLVMIntNE, value, Int8(0));
  LLVMBasicBlockRef body = CurrentBodyBlock();
  LLVMBasicBlockRef end = CreateBlock();
  when(condition, body, end);

  // end
  enter(end);
  pop();
}

/**
 * Bulid command `>` and `<`: move data pointer.
 */
void move(int step) {
  LLVMValueRef pointer = load(LLVMPointerType(LLVMInt8Type(), 0), dp);
  store(dp, Pointer(LLVMInt8Type(), pointer, 1, (LLVMValueRef[]){ Int32(step) }));
}

/**
 * Build command `+` and `-`: apply offset to value of the data pointer.
 */
void update(int offset) {
  LLVMValueRef value = get();
  if (offset > 0) {
    value = add(value, Int8(offset));
  } else if (offset < 0) {
    value = sub(value, Int8(-offset));
  }
  set(value);
}

/**
 * Build command ','.
 */
void input(void) {
  LLVMValueRef value = invoke(s_getchar, 0, (LLVMValueRef[]){});
  value = invoke(s_max, 2, (LLVMValueRef[]){ value, Int32(0) });
  value = truncate(value, LLVMInt8Type());
  set(value);
}

/**
 * Build command '.'.
 */
void output(void) {
  LLVMValueRef value = get();
  LLVMValueRef charactor = extend(value, LLVMInt32Type());
  invoke(s_putchar, 1, (LLVMValueRef[]){ charactor });
}
