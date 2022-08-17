#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <llvm-c/Core.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>

#define SOURCE_FILE_NAME "main.bf"
#define TARGET_DATA_LAYOUT "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"

int main(int argc, char* argv[]) {
  LLVMContextRef context = LLVMContextCreate();
  LLVMModuleRef module = LLVMModuleCreateWithNameInContext("brainfuck", context);
  LLVMSetSourceFileName(module, SOURCE_FILE_NAME, strlen(SOURCE_FILE_NAME));
  LLVMSetDataLayout(module, TARGET_DATA_LAYOUT);
  LLVMSetTarget(module, LLVMGetDefaultTargetTriple());

  // define function

  LLVMTypeRef parameter_types[] = {
    LLVMInt32Type(),
    LLVMInt32Type()
  };
  LLVMTypeRef function_type = LLVMFunctionType(LLVMInt32Type(), parameter_types, 2, 0);
  LLVMValueRef sum = LLVMAddFunction(module, "sum", function_type);
  LLVMBasicBlockRef body = LLVMAppendBasicBlock(sum, "");

  LLVMBuilderRef builder = LLVMCreateBuilder();
  LLVMPositionBuilderAtEnd(builder, body);
  
  LLVMValueRef result = LLVMBuildAdd(builder, LLVMGetParam(sum, 0), LLVMGetParam(sum, 1), "");
  LLVMBuildRet(builder, result);

  printf("%s", LLVMPrintModuleToString(module));

  // emit object file

  LLVMInitializeNativeTarget();
  LLVMInitializeNativeAsmPrinter();
  LLVMInitializeNativeAsmParser();

  char* error = NULL;

  LLVMTargetRef target = NULL;
  if (LLVMGetTargetFromTriple(LLVMGetDefaultTargetTriple(), &target, &error) != 0) {
    fprintf(stderr, "create target failed: %s\n", error);
    LLVMDisposeMessage(error);
    exit(EXIT_FAILURE);
  }
  LLVMTargetMachineRef machine = LLVMCreateTargetMachine(target, LLVMGetDefaultTargetTriple(), LLVMGetHostCPUName(), LLVMGetHostCPUFeatures(), LLVMCodeGenLevelDefault, LLVMRelocDefault, LLVMCodeModelDefault);

  if (LLVMTargetMachineEmitToFile(machine, module, "sum.o", LLVMObjectFile, &error) != 0) {
    fprintf(stderr, "emit object file failed: %s\n", error);
    LLVMDisposeMessage(error);
    exit(EXIT_FAILURE);
  }

  // dispose

  LLVMDisposeBuilder(builder);
  LLVMDisposeModule(module);
  LLVMContextDispose(context);
  LLVMDisposeTargetMachine(machine);
  LLVMShutdown();

  return 0;
}
