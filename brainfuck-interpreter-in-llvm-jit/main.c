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
  s_printf = 0,
  s_sum,
  s_main,
  s_size
} Symbol;

struct {
  LLVMTypeRef types[s_size];
  LLVMValueRef values[s_size];
} SymbolTable;

LLVMValueRef declare(LLVMModuleRef module, Symbol symbol, char* name, LLVMTypeRef type) {
  LLVMValueRef fn = LLVMAddFunction(module, name, type);
  SymbolTable.types[symbol] = type;
  SymbolTable.values[symbol] = fn;
  return fn;
}

LLVMValueRef call(LLVMBuilderRef builder, Symbol symbol, int count, LLVMValueRef parameters[]) {
  LLVMTypeRef type = SymbolTable.types[symbol];
  LLVMValueRef fn = SymbolTable.values[symbol];
  return LLVMBuildCall2(builder, type, fn, parameters, count, "");
}

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

void declare_printf(LLVMModuleRef module) {
  declare(module, s_printf, "printf", LLVMFunctionType(LLVMInt32Type(), (LLVMTypeRef[]){
    LLVMPointerType(LLVMInt8Type(), 0)
  }, 1, 1));
}

void define_sum(LLVMModuleRef module, LLVMBuilderRef builder) {
  LLVMValueRef sum = declare(module, s_sum, "sum", LLVMFunctionType(LLVMInt32Type(), (LLVMTypeRef[]){
    LLVMInt32Type(),
    LLVMInt32Type()
  }, 2, 0));
  LLVMBasicBlockRef body = LLVMAppendBasicBlock(sum, "");
  LLVMPositionBuilderAtEnd(builder, body);

  LLVMValueRef result = LLVMBuildAdd(builder, LLVMGetParam(sum, 0), LLVMGetParam(sum, 1), "");
  LLVMBuildRet(builder, result);
}

void define_main(
  LLVMModuleRef module,
  LLVMBuilderRef builder
) {
  LLVMValueRef main = declare(module, s_main, "main", LLVMFunctionType(LLVMInt32Type(), (LLVMTypeRef[]){}, 0, 0));
  LLVMBasicBlockRef body = LLVMAppendBasicBlock(main, "");
  LLVMPositionBuilderAtEnd(builder, body);

  LLVMValueRef result = call(builder, s_sum, 2, (LLVMValueRef[]){
    LLVMConstInt(LLVMInt32Type(), 1, FALSE),
    LLVMConstInt(LLVMInt32Type(), 2, FALSE)
  });
  call(builder, s_printf, 2, (LLVMValueRef[]){
    LLVMBuildGlobalStringPtr(builder, "sum of 1 + 2 = %d!\n", ""),
    result
  });

  LLVMBuildRet(builder, LLVMConstInt(LLVMInt32Type(), 0, FALSE));
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
