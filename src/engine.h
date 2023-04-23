#ifndef __ENGINE_H_
#define __ENGINE_H_

#include <llvm-c/Core.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/ExecutionEngine.h>

#define EMPTY_SPACE 0
#define Int8PointerType LLVMPointerType(LLVMInt8Type(), EMPTY_SPACE)

#define Int32(n) LLVMConstInt(LLVMInt32Type(), (n), false)
#define Int8(n) LLVMConstInt(LLVMInt8Type(), (n), false)

void TearDownEngine(void);
void SetUpEngine();
void SetDefaultModule(char*);

LLVMValueRef DeclareGlobalVariable(char*, LLVMTypeRef);
LLVMValueRef DeclareGlobalVariableWithValue(char*, LLVMTypeRef, LLVMValueRef);
LLVMValueRef DeclareFunction(char*, LLVMTypeRef);
LLVMValueRef DeclareExternalFunction(char*, LLVMTypeRef, void*);

LLVMValueRef CreateZeroInitializer(LLVMTypeRef, int);

LLVMValueRef CallFunction(LLVMTypeRef, LLVMValueRef, int, LLVMValueRef*);
LLVMBasicBlockRef CreateAndAppendBlock(LLVMValueRef);
void EnterBlock(LLVMBasicBlockRef);

LLVMValueRef GetPointer(LLVMTypeRef, LLVMValueRef, int, LLVMValueRef*);
LLVMValueRef Alloc(LLVMTypeRef);
LLVMValueRef Load(LLVMTypeRef, LLVMValueRef);
void Store(LLVMValueRef, LLVMValueRef);

LLVMValueRef Add(LLVMValueRef, LLVMValueRef);
LLVMValueRef Sub(LLVMValueRef, LLVMValueRef);
LLVMValueRef Compare(LLVMIntPredicate, LLVMValueRef, LLVMValueRef);

void If(LLVMValueRef, LLVMBasicBlockRef, LLVMBasicBlockRef);
void Goto(LLVMBasicBlockRef);
void Return(LLVMValueRef);
void ReturnVoid();

LLVMValueRef ExtendType(LLVMValueRef, LLVMTypeRef);
LLVMValueRef TruncateType(LLVMValueRef, LLVMTypeRef);

void EmitIntermediateRepresentation(char*);
void EmitObjectFile(char*);
void ExecuteMachineCode(void);

#endif
