/**
 * Brainfuck language command builder.
 */
#include <stdlib.h>
#include <stdio.h>

#include "engine.h"
#include "compiler.h"

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
static void StackPush(LLVMBasicBlockRef entry, LLVMBasicBlockRef body) {
  Stack top = (Stack)calloc(sizeof(struct _Stack), 1);
  top->entry = entry;
  top->body = body;
  top->next = stack;
  stack = top;
}

/**
 * Pop the top block.
 */
static void StackPop() {
  if (stack != NULL) {
    Stack top = stack;
    stack = stack->next;
    free(top);
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
static LLVMValueRef DefineFunction(Symbol symbol, char* name, LLVMTypeRef type) {
  SymbolTable.types[symbol] = type;
  SymbolTable.values[symbol] = DeclareFunction(name, type);
  return SymbolTable.values[symbol];
}

/**
 * Call imported function.
 */
static LLVMValueRef InvokeFunction(Symbol symbol, int length, LLVMValueRef* parameters) {
  LLVMTypeRef type = SymbolTable.types[symbol];
  LLVMValueRef fn = SymbolTable.values[symbol];
  return CallFunction(type, fn, length, parameters);
}

/* Data Pointer */
static LLVMValueRef dp = NULL;

/**
 * Get value of the data pointer.
 */
static LLVMValueRef GetValue() {
  LLVMValueRef pointer = Load(LLVMPointerType(LLVMInt8Type(), 0), dp);
  return Load(LLVMInt8Type(), pointer);
}

/**
 * Set value to the data pointer.
 */
static void SetValue(LLVMValueRef value) {
  LLVMValueRef pointer = Load(LLVMPointerType(LLVMInt8Type(), 0), dp);
  Store(pointer, value);
}

/**
 * Create global data segment and return the data pointer.
 */
static LLVMValueRef DefineDataSegment() {
  LLVMTypeRef type = LLVMArrayType(LLVMInt8Type(), DATA_SEGMENT_SIZE);
  LLVMValueRef initializer = CreateZeroInitializer(LLVMInt8Type(), DATA_SEGMENT_SIZE);
  LLVMValueRef ds = DeclareGlobalVariableWithValue("ds", type, initializer);
  return GetPointer(type, ds, 2, (LLVMValueRef[]){ Int32(0), Int32(0) });
}

/**
 * Create basic block and append to main function.
 */
static LLVMBasicBlockRef NewBlock() {
  LLVMValueRef fn = SymbolTable.values[s_main];
  return CreateAndAppendBlock(fn);
}

/* Eight Commands */

/**
 * Build command `[`: while loop begin.
 */
static void WhileNotZero(void) {
  LLVMBasicBlockRef entry = NewBlock();
  Goto(entry);

  LLVMBasicBlockRef body = NewBlock();
  StackPush(entry, body);
  EnterBlock(body);
}

/**
 * Build command `]`: while loop end.
 */
static void WhileEnd(void) {
  // body
  LLVMBasicBlockRef entry = CurrentEntryBlock();
  Goto(entry);

  // entry
  EnterBlock(entry);
  LLVMValueRef value = GetValue();
  LLVMValueRef condition = Compare(LLVMIntNE, value, Int8(0));
  LLVMBasicBlockRef body = CurrentBodyBlock();
  LLVMBasicBlockRef end = NewBlock();
  If(condition, body, end);

  // end
  EnterBlock(end);
  StackPop();
}

/**
 * Bulid command `>` and `<`: move data pointer.
 */
static void MovePointer(int step) {
  LLVMValueRef pointer = Load(LLVMPointerType(LLVMInt8Type(), 0), dp);
  Store(dp, GetPointer(LLVMInt8Type(), pointer, 1, (LLVMValueRef[]){ Int32(step) }));
}

/**
 * Build command `+` and `-`: apply offset to value of the data pointer.
 */
static void UpdateValue(int offset) {
  LLVMValueRef value = GetValue();
  if (offset > 0) {
    value = Add(value, Int8(offset));
  } else if (offset < 0) {
    value = Sub(value, Int8(-offset));
  }
  SetValue(value);
}

/**
 * Build command ','.
 */
static void InputValue(void) {
  LLVMValueRef value = InvokeFunction(s_getchar, 0, (LLVMValueRef[]){});
  value = InvokeFunction(s_max, 2, (LLVMValueRef[]){ value, Int32(0) });
  value = TruncateType(value, LLVMInt8Type());
  SetValue(value);
}

/**
 * Build command '.'.
 */
static void OutputValue(void) {
  LLVMValueRef value = GetValue();
  LLVMValueRef charactor = ExtendType(value, LLVMInt32Type());
  InvokeFunction(s_putchar, 1, (LLVMValueRef[]){ charactor });
}

/* Compiler */

/**
 * Remove all blocks.
 */
static void TearDownCompiler(void) {
  while (stack != NULL) {
    StackPop();
  }
}

/**
 * Setup brainfuck skeleton.
 * - declare getchar.
 * - declare putchar.
 * - declare llvm.smax.i32.
 * - declare global data segment.
 * - create data pointer.
 */
void SetUpCompiler(void) {
  atexit(TearDownCompiler);
  SetUpEngine();
}

/**
 * Compile to default module.
 */
void Compile(char* source) {
  SetDefaultModule(source);

  // Global Variables
  LLVMValueRef ds = DefineDataSegment();

  // Global Functions
  DefineFunction(s_getchar, "getchar", LLVMFunctionType(LLVMInt32Type(), (LLVMTypeRef[]){}, 0, False));
  DefineFunction(s_putchar, "putchar", LLVMFunctionType(LLVMInt32Type(), (LLVMTypeRef[]){ LLVMInt32Type() }, 1, False));
  DefineFunction(s_max, "llvm.smax.i32", LLVMFunctionType(LLVMInt32Type(), (LLVMTypeRef[]){ LLVMInt32Type(), LLVMInt32Type() }, 2, False));

  // Main Begin
  DefineFunction(s_main, "main", LLVMFunctionType(LLVMInt32Type(), (LLVMTypeRef[]){}, 0, False));
  EnterBlock(NewBlock());

  dp = Alloc(LLVMPointerType(LLVMInt8Type(), 0));
  Store(dp, ds);

  // Main Body
  FILE* fin = fopen(source, "r");
  if (fin == NULL) {
    fprintf(stderr, "Open source file %s failed!\n", source);
    exit(EXIT_FAILURE);
  }

  int command = 0;
  while ((command = fgetc(fin)) != EOF) {
    switch (command) {
    case '>':
      MovePointer(1);
      break;
    case '<':
      MovePointer(-1);
      break;
    case '+':
      UpdateValue(1);
      break;
    case '-':
      UpdateValue(-1);
      break;
    case ',':
      InputValue();
      break;
    case '.':
      OutputValue();
      break;
    case '[':
      WhileNotZero();
      break;
    case ']':
      WhileEnd();
      break;
    default:
      /* Ignore Unknown command */
      break;
    }
  }
  fclose(fin);

  // Main End
  Return(Int32(0));
}
