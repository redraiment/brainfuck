#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <llvm-c/Core.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>

#define FALSE 0
#define TRUE 1
#define EMPTY_SPACE 0

#define DATA_SEGMENT_SIZE 30000

/* LLVM Wrap */

#define safe(expression) do {                         \
    char* message = NULL;                             \
    if ((expression) != FALSE) {                      \
      fprintf(stderr, "%s\n", message);               \
      LLVMDisposeMessage(message);                    \
      exit(EXIT_FAILURE);                             \
    }                                                 \
  } while(FALSE)

#define Int32(n) LLVMConstInt(LLVMInt32Type(), (n), FALSE)
#define Int32Array(n) LLVMArrayType(LLVMInt32Type(), (n))
#define Int8(n) LLVMConstInt(LLVMInt8Type(), (n), FALSE)
#define Int8Array(n) LLVMArrayType(LLVMInt8Type(), (n))

static LLVMTargetMachineRef machine = NULL;
static LLVMTargetDataRef layout = NULL;
static LLVMModuleRef module = NULL;
static LLVMBuilderRef builder = NULL;

typedef enum {
  s_getchar = 0,
  s_putchar,
  s_main,
  s_size
} Symbol;

struct {
  LLVMTypeRef types[s_size];
  LLVMValueRef values[s_size];
} SymbolTable;

LLVMValueRef declare(Symbol symbol, char* name, LLVMTypeRef type) {
  LLVMValueRef fn = LLVMAddFunction(module, name, type);
  SymbolTable.types[symbol] = type;
  SymbolTable.values[symbol] = fn;
  return fn;
}

LLVMValueRef call(Symbol symbol, int count, LLVMValueRef parameters[]) {
  LLVMTypeRef type = SymbolTable.types[symbol];
  LLVMValueRef fn = SymbolTable.values[symbol];
  return LLVMBuildCall2(builder, type, fn, parameters, count, "");
}

void LLVMInitialize(char* sourceFileName) {
  // Initialize target machine
  LLVMInitializeNativeTarget();
  LLVMInitializeNativeAsmPrinter();
  LLVMInitializeNativeAsmParser();

  LLVMTargetRef target = NULL;
  safe(LLVMGetTargetFromTriple(LLVMGetDefaultTargetTriple(), &target, &message));

  machine = LLVMCreateTargetMachine(target, LLVMGetDefaultTargetTriple(), LLVMGetHostCPUName(), LLVMGetHostCPUFeatures(), LLVMCodeGenLevelDefault, LLVMRelocDefault, LLVMCodeModelDefault);

  layout = LLVMCreateTargetDataLayout(machine);

  module = LLVMModuleCreateWithName(sourceFileName);
  LLVMSetSourceFileName(module, sourceFileName, strlen(sourceFileName));
  LLVMSetTarget(module, LLVMGetDefaultTargetTriple());
  LLVMSetDataLayout(module, LLVMCopyStringRepOfTargetData(layout));

  builder = LLVMCreateBuilder();
}

void LLVMDispose() {
  if (builder != NULL) {
    LLVMDisposeBuilder(builder);
  }
  if (module != NULL) {
    LLVMDisposeModule(module);
  }
  if (layout != NULL) {
    LLVMDisposeTargetData(layout);
  }
  if (machine != NULL) {
    LLVMDisposeTargetMachine(machine);
  }
  LLVMShutdown();
}

void DeclareGetchar() {
  declare(s_getchar, "getchar", LLVMFunctionType(LLVMInt32Type(), (LLVMTypeRef[]){}, 0, FALSE));
}

void DeclarePutchar() {
  declare(s_putchar, "putchar", LLVMFunctionType(LLVMInt32Type(), (LLVMTypeRef[]){ LLVMInt32Type() }, 1, FALSE));
}

void define_main(
  LLVMModuleRef module,
  LLVMBuilderRef builder
) {
  LLVMValueRef main = declare(s_main, "main", LLVMFunctionType(LLVMInt32Type(), (LLVMTypeRef[]){}, 0, FALSE));
  LLVMBasicBlockRef body = LLVMAppendBasicBlock(main, "");
  LLVMPositionBuilderAtEnd(builder, body);

  LLVMBuildRet(builder, Int32(0));
}

void emit_object_file(char* sourceFileName) {
  char* end = strrchr(sourceFileName, '.');
  int length = end != NULL? (end - sourceFileName) : strlen(sourceFileName);
  char objectFileName[length + 2];
  strncpy(objectFileName, sourceFileName, length);
  objectFileName[length] = '.';
  objectFileName[length + 1] = 'o';
  objectFileName[length + 2] = 0;

  safe(LLVMTargetMachineEmitToFile(machine, module, objectFileName, LLVMObjectFile, &message));
}

/**
 * Creates global data segment and returns the data pointer.
 */
LLVMValueRef CreateDataSegment() {
  LLVMTypeRef type = Int8Array(DATA_SEGMENT_SIZE);
  LLVMValueRef ds = LLVMAddGlobal(module, type, "ds");
  LLVMValueRef array[DATA_SEGMENT_SIZE];
  for (int index = 0; index < DATA_SEGMENT_SIZE; index++) {
    array[index] = Int8(0);
  }
  LLVMValueRef zeroinitializer = LLVMConstArray(LLVMInt8Type(), array, DATA_SEGMENT_SIZE);
  LLVMSetInitializer(ds, zeroinitializer);

  return LLVMBuildGEP2(builder, type, ds, (LLVMValueRef[]){ Int32(0), Int32(0) }, 2, "");
}

/**
 * Move data pointer.
 */
LLVMValueRef shift(LLVMValueRef dp, int offset) {
  return LLVMBuildGEP2(builder, LLVMInt8Type(), dp, (LLVMValueRef[]){
      Int32(offset)
  }, 1, "");
}

void update(LLVMValueRef dp, int offset) {
  LLVMValueRef value = LLVMBuildLoad2(builder, LLVMInt8Type(), dp, "");
  if (offset > 0) {
    value = LLVMBuildAdd(builder, value, Int8(offset), "");
  } else if (offset < 0) {
    value = LLVMBuildSub(builder, value, Int8(-offset), "");
  }
  LLVMBuildStore(builder, value, dp);
}

void output(LLVMValueRef dp) {
  LLVMValueRef value = LLVMBuildLoad2(builder, LLVMInt8Type(), dp, "");
  LLVMValueRef charactor = LLVMBuildSExt(builder, value, LLVMInt32Type(), "");
  call(s_putchar, 1, (LLVMValueRef[]){ charactor });
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <source-file>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  // initialize
  LLVMInitialize(argv[1]);
  DeclareGetchar();
  DeclarePutchar();
  LLVMValueRef dp = CreateDataSegment();

  // main {
  LLVMValueRef main = declare(s_main, "main", LLVMFunctionType(LLVMInt32Type(), (LLVMTypeRef[]){}, 0, FALSE));
  LLVMPositionBuilderAtEnd(builder, LLVMAppendBasicBlock(main, ""));

  update(dp, 10);

  // while {
  LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main, "");
  LLVMBasicBlockRef body = LLVMAppendBasicBlock(main, "");
  LLVMBasicBlockRef end = LLVMAppendBasicBlock(main, "");
  LLVMBuildBr(builder, entry);

  LLVMPositionBuilderAtEnd(builder, entry);
  LLVMValueRef value = LLVMBuildLoad2(builder, LLVMInt8Type(), dp, "");
  LLVMValueRef condition = LLVMBuildICmp(builder, LLVMIntNE, value, Int8(0), "");
  LLVMBuildCondBr(builder, condition, body, end);

  LLVMPositionBuilderAtEnd(builder, body);
  dp = shift(dp, 1);
  update(dp, 7);

  dp = shift(dp, 1);
  update(dp, 10);
  
  dp = shift(dp, 1);
  update(dp, 3);
  
  dp = shift(dp, 1);
  update(dp, 1);
  
  dp = shift(dp, -4);
  update(dp, -1);
  LLVMBuildBr(builder, entry);

  // while }
  LLVMPositionBuilderAtEnd(builder, end);

  // H
  dp = shift(dp, 1);
  update(dp, 2);
  output(dp);

  // e
  dp = shift(dp, 1);
  update(dp, 1);
  output(dp);

  // l
  update(dp, 7);
  output(dp);

  // l
  output(dp);

  // o
  update(dp, 3);
  output(dp);

  // \space
  dp = shift(dp, 1);
  update(dp, 2);
  output(dp);

  // W
  dp = shift(dp, -2);
  update(dp, 15);
  output(dp);

  // o
  dp = shift(dp, 1);
  output(dp);

  // r
  update(dp, 3);
  output(dp);

  // l
  update(dp, -6);
  output(dp);

  // d
  update(dp, -8);
  output(dp);

  // !
  dp = shift(dp, 1);
  update(dp, 1);
  output(dp);

  // \newline
  dp = shift(dp, 1);
  output(dp);

  // main }
  LLVMBuildRet(builder, Int32(0));

  // output
  emit_object_file(argv[1]);

  LLVMDispose();

  return 0;
}
