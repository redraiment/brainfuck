#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <llvm-c/Core.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>

#define SOURCE_FILE_NAME "main.bf"

#define safe(expression, prompt) do {                 \
    char* message = NULL;                             \
    if ((expression) != 0) {                          \
      fprintf(stderr, "%s:%s\n", (prompt), message);  \
      LLVMDisposeMessage(message);                    \
      exit(EXIT_FAILURE);                             \
    }                                                 \
  } while(0)

int main(int argc, char* argv[]) {
  // Initialize target machine
  LLVMInitializeNativeTarget();
  LLVMInitializeNativeAsmPrinter();
  LLVMInitializeNativeAsmParser();

  LLVMTargetRef target = NULL;
  safe(LLVMGetTargetFromTriple(LLVMGetDefaultTargetTriple(), &target, &message), "create target failed");
  LLVMTargetMachineRef machine = LLVMCreateTargetMachine(target, LLVMGetDefaultTargetTriple(), LLVMGetHostCPUName(), LLVMGetHostCPUFeatures(), LLVMCodeGenLevelDefault, LLVMRelocDefault, LLVMCodeModelDefault);
  LLVMTargetDataRef layout = LLVMCreateTargetDataLayout(machine);

  // Initialize Module
  LLVMModuleRef module = LLVMModuleCreateWithName("brainfuck");
  LLVMSetSourceFileName(module, SOURCE_FILE_NAME, strlen(SOURCE_FILE_NAME));
  LLVMSetTarget(module, LLVMGetDefaultTargetTriple());
  LLVMSetDataLayout(module, LLVMCopyStringRepOfTargetData(layout));

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
  safe(LLVMTargetMachineEmitToFile(machine, module, "sum.o", LLVMObjectFile, &message), "emit object file failed");

  // dispose
  LLVMDisposeTargetData(layout);
  LLVMDisposeTargetMachine(machine);
  LLVMDisposeBuilder(builder);
  LLVMDisposeModule(module);
  LLVMShutdown();

  return 0;
}
