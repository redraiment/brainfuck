#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <llvm-c/Core.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>

#define FALSE 0
#define TRUE 1

#define SOURCE_FILE_NAME "sum.ll"

#define safe(expression, prompt) do {                 \
    char* message = NULL;                             \
    if ((expression) != 0) {                          \
      fprintf(stderr, "%s:%s\n", (prompt), message);  \
      LLVMDisposeMessage(message);                    \
      exit(EXIT_FAILURE);                             \
    }                                                 \
  } while(0)

LLVMTargetMachineRef create_target_machine() {
  // Initialize target machine
  LLVMInitializeNativeTarget();
  LLVMInitializeNativeAsmPrinter();
  LLVMInitializeNativeAsmParser();

  LLVMTargetRef target = NULL;
  safe(LLVMGetTargetFromTriple(LLVMGetDefaultTargetTriple(), &target, &message), "create target failed");
  return LLVMCreateTargetMachine(target, LLVMGetDefaultTargetTriple(), LLVMGetHostCPUName(), LLVMGetHostCPUFeatures(), LLVMCodeGenLevelDefault, LLVMRelocDefault, LLVMCodeModelDefault);
}

LLVMModuleRef create_module(LLVMTargetDataRef layout) {
  // Initialize Module
  LLVMModuleRef module = LLVMModuleCreateWithName(SOURCE_FILE_NAME);
  LLVMSetSourceFileName(module, SOURCE_FILE_NAME, strlen(SOURCE_FILE_NAME));
  LLVMSetTarget(module, LLVMGetDefaultTargetTriple());
  LLVMSetDataLayout(module, LLVMCopyStringRepOfTargetData(layout));
  return module;
}

LLVMValueRef declare_printf(LLVMModuleRef module) {
  LLVMTypeRef arguments[] = { LLVMPointerType(LLVMInt8Type(), 0) };
  LLVMTypeRef type = LLVMFunctionType(LLVMInt32Type(), arguments, 1, 1);
  return LLVMAddFunction(module, "printf", type);
}

LLVMValueRef define_sum(LLVMModuleRef module, LLVMBuilderRef builder) {
  LLVMTypeRef arguments[] = { LLVMInt32Type(), LLVMInt32Type() };
  LLVMTypeRef type = LLVMFunctionType(LLVMInt32Type(), arguments, 2, 0);
  LLVMValueRef sum = LLVMAddFunction(module, "sum", type);
  LLVMBasicBlockRef body = LLVMAppendBasicBlock(sum, "");

  LLVMPositionBuilderAtEnd(builder, body);
  LLVMValueRef result = LLVMBuildAdd(builder, LLVMGetParam(sum, 0), LLVMGetParam(sum, 1), "");
  LLVMBuildRet(builder, result);

  return sum;
}

LLVMValueRef call_sum(
  LLVMBuilderRef builder,
  LLVMValueRef sum,
  int left,
  int right
) {
  LLVMTypeRef arguments[] = { LLVMInt32Type(), LLVMInt32Type() };
  LLVMTypeRef type = LLVMFunctionType(LLVMInt32Type(), arguments, 2, 0);
  LLVMValueRef parameters[] = {
    LLVMConstInt(LLVMInt32Type(), left, FALSE),
    LLVMConstInt(LLVMInt32Type(), right, FALSE)
  };
  return LLVMBuildCall2(builder, type, sum, parameters, 2, "");
}

LLVMValueRef call_printf(
  LLVMBuilderRef builder,
  LLVMValueRef printf,
  char* template,
  LLVMValueRef result
) {
  LLVMTypeRef arguments[] = { LLVMPointerType(LLVMInt8Type(), 0) };
  LLVMTypeRef type = LLVMFunctionType(LLVMInt32Type(), arguments, 1, 1);
  LLVMValueRef parameters[] = {
    LLVMBuildGlobalStringPtr(builder, template, ""),
    result
  };
  return LLVMBuildCall2(builder, type, printf, parameters, 2, "");
}

LLVMValueRef define_main(
  LLVMModuleRef module,
  LLVMBuilderRef builder,
  LLVMValueRef fn_printf,
  LLVMValueRef fn_sum
) {
  LLVMTypeRef arguments[] = {};
  LLVMTypeRef type = LLVMFunctionType(LLVMInt32Type(), arguments, 0, 0);
  LLVMValueRef main = LLVMAddFunction(module, "main", type);
  LLVMBasicBlockRef body = LLVMAppendBasicBlock(main, "");

  LLVMPositionBuilderAtEnd(builder, body);
  LLVMValueRef result = call_sum(builder, fn_sum, 1, 2);
  call_printf(builder, fn_printf, "sum of 1 + 2 = %d!\n", result);

  LLVMBuildRet(builder, LLVMConstInt(LLVMInt32Type(), 0, FALSE));
  return main;
}

int main(int argc, char* argv[]) {
  LLVMTargetMachineRef machine = create_target_machine();
  LLVMTargetDataRef layout = LLVMCreateTargetDataLayout(machine);
  LLVMModuleRef module = create_module(layout);

  LLVMValueRef fn_printf = declare_printf(module);

  LLVMBuilderRef builder = LLVMCreateBuilder();
  LLVMValueRef fn_sum = define_sum(module, builder);
  define_main(module, builder, fn_printf, fn_sum);

  printf("%s", LLVMPrintModuleToString(module));

  // emit object file
  safe(LLVMTargetMachineEmitToFile(machine, module, "hello.o", LLVMObjectFile, &message), "emit object file failed");

  // dispose
  LLVMDisposeBuilder(builder);
  LLVMDisposeModule(module);
  LLVMDisposeTargetData(layout);
  LLVMDisposeTargetMachine(machine);
  LLVMShutdown();

  return 0;
}
