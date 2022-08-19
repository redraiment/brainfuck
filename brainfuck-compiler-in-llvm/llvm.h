#ifndef __LLVM_H_
#define __LLVM_H_

#include <llvm-c/Core.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>

#define False 0
#define True 1

#define Int32(n) LLVMConstInt(LLVMInt32Type(), (n), False)
#define Int8(n) LLVMConstInt(LLVMInt8Type(), (n), False)

void LLVMSetUp(char*);

LLVMValueRef DeclareGlobalVariable(char*, LLVMTypeRef);
LLVMValueRef DeclareGlobalVariableWithValue(char*, LLVMTypeRef, LLVMValueRef);
LLVMValueRef DeclareFunction(char*, LLVMTypeRef);

LLVMValueRef ConstZeroArray(LLVMTypeRef, int);

LLVMValueRef call(LLVMTypeRef, LLVMValueRef, int, LLVMValueRef*);
LLVMBasicBlockRef Block(LLVMValueRef);
void enter(LLVMBasicBlockRef);

LLVMValueRef Pointer(LLVMTypeRef, LLVMValueRef, int, LLVMValueRef*);
LLVMValueRef alloc(LLVMTypeRef);
LLVMValueRef load(LLVMTypeRef, LLVMValueRef);
void store(LLVMValueRef, LLVMValueRef);

LLVMValueRef add(LLVMValueRef, LLVMValueRef);
LLVMValueRef sub(LLVMValueRef, LLVMValueRef);
LLVMValueRef compare(LLVMIntPredicate, LLVMValueRef, LLVMValueRef);

void when(LLVMValueRef, LLVMBasicBlockRef, LLVMBasicBlockRef);
void jumpTo(LLVMBasicBlockRef);
void returnWith(LLVMValueRef);
void returnVoid();

LLVMValueRef extend(LLVMValueRef, LLVMTypeRef);
LLVMValueRef truncate(LLVMValueRef, LLVMTypeRef);

void EmitObjectFile(char*);

#endif
