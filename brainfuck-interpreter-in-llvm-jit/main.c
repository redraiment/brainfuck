#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <llvm-c/Core.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>

#define FALSE 0
#define TRUE 1

#define PROGRAM "sum"
#define SOURCE "sum.c"
#define OBJECT "sum.o"

#define LINKER "/bin/ld"
#define LOADER "/lib64/ld-linux-x86-64.so.2"
#define RUNTIME_START "/lib/Scrt1.o"
#define RUNTIME_INIT "/lib/crti.o"
#define RUNTIME_FINI "/lib/crtn.o"

#define safe(expression) do {                         \
    char* message = NULL;                             \
    if ((expression) != 0) {                          \
      fprintf(stderr, "%s\n", message);               \
      LLVMDisposeMessage(message);                    \
      exit(EXIT_FAILURE);                             \
    }                                                 \
  } while(0)

typedef enum {
  symbol_printf = 0,
  symbol_sum,
  symbol_main,
  symbol_size
} Symbols;

struct {
  LLVMTypeRef types[symbol_size];
  LLVMValueRef values[symbol_size];
} SymbolTable;

LLVMTargetMachineRef create_target_machine() {
  // Initialize target machine
  LLVMInitializeNativeTarget();
  LLVMInitializeNativeAsmPrinter();
  LLVMInitializeNativeAsmParser();

  LLVMTargetRef target = NULL;
  safe(LLVMGetTargetFromTriple(LLVMGetDefaultTargetTriple(), &target, &message));
  return LLVMCreateTargetMachine(target, LLVMGetDefaultTargetTriple(), LLVMGetHostCPUName(), LLVMGetHostCPUFeatures(), LLVMCodeGenLevelDefault, LLVMRelocDefault, LLVMCodeModelDefault);
}

LLVMModuleRef create_module(LLVMTargetDataRef layout) {
  // Initialize Module
  LLVMModuleRef module = LLVMModuleCreateWithName(PROGRAM);
  LLVMSetSourceFileName(module, SOURCE, strlen(SOURCE));
  LLVMSetTarget(module, LLVMGetDefaultTargetTriple());
  LLVMSetDataLayout(module, LLVMCopyStringRepOfTargetData(layout));
  return module;
}

LLVMValueRef declare_printf(LLVMModuleRef module) {
  LLVMTypeRef arguments[] = { LLVMPointerType(LLVMInt8Type(), 0) };
  LLVMTypeRef type = LLVMFunctionType(LLVMInt32Type(), arguments, 1, 1);
  LLVMValueRef printf = LLVMAddFunction(module, "printf", type);

  SymbolTable.types[symbol_printf] = type;
  SymbolTable.values[symbol_printf] = printf;

  return printf;
}

LLVMValueRef define_sum(LLVMModuleRef module, LLVMBuilderRef builder) {
  LLVMTypeRef arguments[] = { LLVMInt32Type(), LLVMInt32Type() };
  LLVMTypeRef type = LLVMFunctionType(LLVMInt32Type(), arguments, 2, 0);
  LLVMValueRef sum = LLVMAddFunction(module, "sum", type);
  LLVMBasicBlockRef body = LLVMAppendBasicBlock(sum, "");

  LLVMPositionBuilderAtEnd(builder, body);
  LLVMValueRef result = LLVMBuildAdd(builder, LLVMGetParam(sum, 0), LLVMGetParam(sum, 1), "");
  LLVMBuildRet(builder, result);

  SymbolTable.types[symbol_sum] = type;
  SymbolTable.values[symbol_sum] = sum;

  return sum;
}

LLVMValueRef call_sum(
  LLVMBuilderRef builder,
  int left,
  int right
) {
  LLVMTypeRef type = SymbolTable.types[symbol_sum];
  LLVMValueRef sum = SymbolTable.values[symbol_sum];
  LLVMValueRef parameters[] = {
    LLVMConstInt(LLVMInt32Type(), left, FALSE),
    LLVMConstInt(LLVMInt32Type(), right, FALSE)
  };
  return LLVMBuildCall2(builder, type, sum, parameters, 2, "");
}

LLVMValueRef call_printf(
  LLVMBuilderRef builder,
  char* template,
  LLVMValueRef result
) {
  LLVMTypeRef type = SymbolTable.types[symbol_printf];
  LLVMValueRef printf = SymbolTable.values[symbol_printf];
  LLVMValueRef parameters[] = {
    LLVMBuildGlobalStringPtr(builder, template, ""),
    result
  };
  return LLVMBuildCall2(builder, type, printf, parameters, 2, "");
}

LLVMValueRef define_main(
  LLVMModuleRef module,
  LLVMBuilderRef builder
) {
  LLVMTypeRef arguments[] = {};
  LLVMTypeRef type = LLVMFunctionType(LLVMInt32Type(), arguments, 0, 0);
  LLVMValueRef main = LLVMAddFunction(module, "main", type);
  LLVMBasicBlockRef body = LLVMAppendBasicBlock(main, "");

  LLVMPositionBuilderAtEnd(builder, body);
  LLVMValueRef result = call_sum(builder, 1, 2);
  call_printf(builder, "sum of 1 + 2 = %d!\n", result);

  LLVMBuildRet(builder, LLVMConstInt(LLVMInt32Type(), 0, FALSE));
  return main;
}

void emit_object_file(LLVMTargetMachineRef machine, LLVMModuleRef module) {
#ifdef DEBUG
  printf("%s", LLVMPrintModuleToString(module));
#endif
  safe(LLVMTargetMachineEmitToFile(machine, module, OBJECT, LLVMObjectFile, &message));
}

void link() {
  char cmd[512] = {0};
  sprintf(cmd,
          "%s -dynamic-linker %s -lc %s %s %s %s -o %s",
          LINKER, LOADER,
          RUNTIME_START, RUNTIME_INIT, OBJECT, RUNTIME_FINI,
          PROGRAM);
  system(cmd);
}

int main(int argc, char* argv[]) {
  // initialize
  LLVMTargetMachineRef machine = create_target_machine();
  LLVMTargetDataRef layout = LLVMCreateTargetDataLayout(machine);
  LLVMModuleRef module = create_module(layout);
  LLVMBuilderRef builder = LLVMCreateBuilder();

  // build
  declare_printf(module);
  define_sum(module, builder);
  define_main(module, builder);

  // output
  emit_object_file(machine, module);
  link();

  // dispose
  LLVMDisposeBuilder(builder);
  LLVMDisposeModule(module);
  LLVMDisposeTargetData(layout);
  LLVMDisposeTargetMachine(machine);
  LLVMShutdown();

  return 0;
}
