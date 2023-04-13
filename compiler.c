/**
 * Brainfuck language command builder.
 */
#include <stdio.h>

#include "engine.h"
#include "scanner.h"
#include "parser.h"
#include "ast.h"
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
static LLVMValueRef DefineFunction(Symbol symbol, char* name, LLVMTypeRef type, void* value) {
  SymbolTable.types[symbol] = type;
  SymbolTable.values[symbol] = value == NULL ? DeclareFunction(name, type) : DeclareExternalFunction(name, type, value);
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
  LLVMValueRef pointer = Load(Int8PointerType, dp);
  return Load(LLVMInt8Type(), pointer);
}

/**
 * Set value to the data pointer.
 */
static void SetValue(LLVMValueRef value) {
  LLVMValueRef pointer = Load(Int8PointerType, dp);
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
void WhileNotZero(void) {
  LLVMBasicBlockRef entry = NewBlock();
  Goto(entry);

  LLVMBasicBlockRef body = NewBlock();
  StackPush(entry, body);
  EnterBlock(body);
}

/**
 * Build command `]`: while loop end.
 */
void WhileEnd(void) {
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
void MovePointer(int step) {
  LLVMValueRef pointer = Load(Int8PointerType, dp);
  Store(dp, GetPointer(LLVMInt8Type(), pointer, 1, (LLVMValueRef[]){ Int32(step) }));
}

/**
 * Build command `+` and `-`: apply offset to value of the data pointer.
 */
void UpdateValue(int offset) {
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
void InputValue(void) {
  LLVMValueRef value = InvokeFunction(s_getchar, 0, (LLVMValueRef[]){});
  value = InvokeFunction(s_max, 2, (LLVMValueRef[]){ value, Int32(0) });
  value = TruncateType(value, LLVMInt8Type());
  SetValue(value);
}

/**
 * Build command '.'.
 */
void OutputValue(void) {
  LLVMValueRef value = GetValue();
  LLVMValueRef charactor = ExtendType(value, LLVMInt32Type());
  InvokeFunction(s_putchar, 1, (LLVMValueRef[]){ charactor });
}

/* Compiler */

/**
 * Remove all blocks.
 */
void TearDownCompiler(void) {
  TearDownEngine();
  while (stack != NULL) {
    StackPop();
  }
  DisposeAst(AstRoot);
}

/**
 * Setup brainfuck skeleton.
 * - declare getchar.
 * - declare putchar.
 * - declare max.
 * - declare global data segment.
 * - create data pointer.
 */
void SetUpCompiler(void) {
  SetUpEngine();
}

/**
 * Compile AST to LLVM IR.
 */
static void CompileAst(Ast ast) {
  if (ast->previous != NULL) {
    CompileAst(ast->previous);
  }
  if (ast->type == BlockNode) {
    WhileNotZero();
    CompileAst(ast->block);
    WhileEnd();
  } else {
    switch (ast->instruction->symbol) {
    case UpdateInstruction:
      UpdateValue(ast->instruction->parameter);
      break;
    case MoveInstruction:
      MovePointer(ast->instruction->parameter);
      break;
    case InputInstruction:
      InputValue();
      break;
    case OutputInstruction:
      OutputValue();
      break;
    default:
      /* Unknown Instruction */
      break;
    }
  }
}

/**
 * Compile to default module.
 */
void Compile(char* source) {
  SetDefaultModule(source);

  // Global Variables
  LLVMValueRef ds = DefineDataSegment();

  // External Functions
  DefineFunction(s_getchar, "getchar", LLVMFunctionType(LLVMInt32Type(), (LLVMTypeRef[]){}, 0, False), getchar);
  DefineFunction(s_putchar, "putchar", LLVMFunctionType(LLVMInt32Type(), (LLVMTypeRef[]){ LLVMInt32Type() }, 1, False), putchar);

  // Global Functions
  LLVMValueRef max = DefineFunction(s_max, "max", LLVMFunctionType(LLVMInt32Type(), (LLVMTypeRef[]){ LLVMInt32Type(), LLVMInt32Type() }, 2, False), NULL);
  EnterBlock(CreateAndAppendBlock(max));
  LLVMValueRef condition = Compare(LLVMIntSGT, LLVMGetParam(max, 0), LLVMGetParam(max, 1));
  LLVMBasicBlockRef then = CreateAndAppendBlock(max);
  LLVMBasicBlockRef otherwise = CreateAndAppendBlock(max);
  If(condition, then, otherwise);
  EnterBlock(then);
  Return(LLVMGetParam(max, 0));
  EnterBlock(otherwise);
  Return(LLVMGetParam(max, 1));

  // Main Begin
  DefineFunction(s_main, "main", LLVMFunctionType(LLVMInt32Type(), (LLVMTypeRef[]){}, 0, False), NULL);
  EnterBlock(NewBlock());

  dp = Alloc(Int8PointerType);
  Store(dp, ds);

  // Main Body
  yyin = fopen(source, "r");
  if (yyin == NULL) {
    fprintf(stderr, "Open source file %s failed!\n", source);
    exit(EXIT_FAILURE);
  }
  yyparse();
  OptimizeAst(AstRoot);
  CompileAst(AstRoot);

  // Main End
  Return(Int32(0));
}
