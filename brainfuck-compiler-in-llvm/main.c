#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "llvm.h"

#define DATA_SEGMENT_SIZE 30000

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
  LLVMValueRef fn = LLVMAddFunction(Module(), name, type);
  SymbolTable.types[symbol] = type;
  SymbolTable.values[symbol] = fn;
  return fn;
}

LLVMValueRef call(Symbol symbol, int count, LLVMValueRef parameters[]) {
  LLVMTypeRef type = SymbolTable.types[symbol];
  LLVMValueRef fn = SymbolTable.values[symbol];
  return LLVMBuildCall2(Builder(), type, fn, parameters, count, "");
}

void DeclareGetchar() {
  declare(s_getchar, "getchar", LLVMFunctionType(LLVMInt32Type(), (LLVMTypeRef[]){}, 0, False));
}

void DeclarePutchar() {
  declare(s_putchar, "putchar", LLVMFunctionType(LLVMInt32Type(), (LLVMTypeRef[]){ LLVMInt32Type() }, 1, False));
}

void EmitObjectFile(char* sourceFileName) {
  char* end = strrchr(sourceFileName, '.');
  int length = end != NULL? (end - sourceFileName) : strlen(sourceFileName);
  char objectFileName[length + 2];
  strncpy(objectFileName, sourceFileName, length);
  objectFileName[length] = '.';
  objectFileName[length + 1] = 'o';
  objectFileName[length + 2] = 0;

  safe(LLVMTargetMachineEmitToFile(TargetMachine(), Module(), objectFileName, LLVMObjectFile, &message));
}

/**
 * Creates global data segment and returns the data pointer.
 */
LLVMValueRef CreateDataSegment() {
  LLVMTypeRef type = Int8Array(DATA_SEGMENT_SIZE);
  LLVMValueRef ds = LLVMAddGlobal(Module(), type, "ds");
  LLVMValueRef array[DATA_SEGMENT_SIZE];
  for (int index = 0; index < DATA_SEGMENT_SIZE; index++) {
    array[index] = Int8(0);
  }
  LLVMValueRef zeroinitializer = LLVMConstArray(LLVMInt8Type(), array, DATA_SEGMENT_SIZE);
  LLVMSetInitializer(ds, zeroinitializer);

  return LLVMBuildGEP2(Builder(), type, ds, (LLVMValueRef[]){ Int32(0), Int32(0) }, 2, "");
}

/**
 * Move data pointer.
 */
LLVMValueRef shift(LLVMValueRef dp, int offset) {
  return LLVMBuildGEP2(Builder(), LLVMInt8Type(), dp, (LLVMValueRef[]){
      Int32(offset)
  }, 1, "");
}

void update(LLVMValueRef dp, int offset) {
  LLVMValueRef value = LLVMBuildLoad2(Builder(), LLVMInt8Type(), dp, "");
  if (offset > 0) {
    value = LLVMBuildAdd(Builder(), value, Int8(offset), "");
  } else if (offset < 0) {
    value = LLVMBuildSub(Builder(), value, Int8(-offset), "");
  }
  LLVMBuildStore(Builder(), value, dp);
}

void output(LLVMValueRef dp) {
  LLVMValueRef value = LLVMBuildLoad2(Builder(), LLVMInt8Type(), dp, "");
  LLVMValueRef charactor = LLVMBuildSExt(Builder(), value, LLVMInt32Type(), "");
  call(s_putchar, 1, (LLVMValueRef[]){ charactor });
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <source-file>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  // initialize
  LLVMSetUp(argv[1]);
  DeclareGetchar();
  DeclarePutchar();
  LLVMValueRef dp = CreateDataSegment();

  // main {
  LLVMValueRef main = declare(s_main, "main", LLVMFunctionType(LLVMInt32Type(), (LLVMTypeRef[]){}, 0, False));
  LLVMPositionBuilderAtEnd(Builder(), LLVMAppendBasicBlock(main, ""));

  update(dp, 10);

  // while {
  LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main, "");
  LLVMBasicBlockRef body = LLVMAppendBasicBlock(main, "");
  LLVMBasicBlockRef end = LLVMAppendBasicBlock(main, "");
  LLVMBuildBr(Builder(), entry);

  LLVMPositionBuilderAtEnd(Builder(), entry);
  LLVMValueRef value = LLVMBuildLoad2(Builder(), LLVMInt8Type(), dp, "");
  LLVMValueRef condition = LLVMBuildICmp(Builder(), LLVMIntNE, value, Int8(0), "");
  LLVMBuildCondBr(Builder(), condition, body, end);

  LLVMPositionBuilderAtEnd(Builder(), body);
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
  LLVMBuildBr(Builder(), entry);

  // while }
  LLVMPositionBuilderAtEnd(Builder(), end);

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
  LLVMBuildRet(Builder(), Int32(0));

  // output
  EmitObjectFile(argv[1]);

  return 0;
}
