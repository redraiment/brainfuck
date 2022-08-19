#ifndef __LLVM_H_
#define __LLVM_H_

#include <llvm-c/Core.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>

#define False 0
#define True 1

#define safe(expression) do {                         \
    char* message = NULL;                             \
    if ((expression) != 0) {                          \
      fprintf(stderr, "%s\n", message);               \
      LLVMDisposeMessage(message);                    \
      exit(EXIT_FAILURE);                             \
    }                                                 \
  } while(False)

#define Int32(n) LLVMConstInt(LLVMInt32Type(), (n), False)
#define Int32Array(n) LLVMArrayType(LLVMInt32Type(), (n))
#define Int8(n) LLVMConstInt(LLVMInt8Type(), (n), False)
#define Int8Array(n) LLVMArrayType(LLVMInt8Type(), (n))

extern LLVMTargetMachineRef machine ;
extern LLVMModuleRef module;
extern LLVMBuilderRef builder;

void LLVMSetUp(char*);

#endif
