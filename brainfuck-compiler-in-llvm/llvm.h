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
#define Int8(n) LLVMConstInt(LLVMInt8Type(), (n), False)

void LLVMSetUp(char*);

LLVMBuilderRef Builder(void);

LLVMValueRef DeclareGlobalVariable(char*, LLVMTypeRef);
LLVMValueRef DeclareGlobalVariableWithValue(char*, LLVMTypeRef, LLVMValueRef);
LLVMValueRef DeclareFunction(char*, LLVMTypeRef);
LLVMValueRef ConstZeroArray(LLVMTypeRef, int);

void EmitObjectFile(char*);

#endif
