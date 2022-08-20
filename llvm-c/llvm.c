/**
 * LLVM Wrap
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "llvm.h"

/* inner default machine. */
static LLVMTargetMachineRef machine = NULL;

/* inner default module. */
static LLVMModuleRef module = NULL;

/* inner default builder. */
static LLVMBuilderRef builder = NULL;

/**
 * Destroy all LLVM resources.
 */
static void LLVMTearDown(void) {
  if (builder != NULL) {
    LLVMDisposeBuilder(builder);
  }
  if (module != NULL) {
    LLVMDisposeModule(module);
  }
  if (machine != NULL) {
    LLVMDisposeTargetMachine(machine);
  }
  LLVMShutdown();
}

/**
 * Initialize LLVM target machine with given source filename.
 * - create target machine.
 * - create module.
 * - create builder.
 */
void LLVMSetUp(char* filename) {
  // Initialize target machine
  LLVMInitializeNativeTarget();
  LLVMInitializeNativeAsmPrinter();
  LLVMInitializeNativeAsmParser();

  char* triple = LLVMGetDefaultTargetTriple();
  LLVMTargetRef target = NULL;
  char* message = NULL;
  if (LLVMGetTargetFromTriple(triple, &target, &message) != 0) {
    fprintf(stderr, "LLVM get target from triple failed: %s\n", message);
    LLVMDisposeMessage(message);
    exit(EXIT_FAILURE);
  }

  machine = LLVMCreateTargetMachine(target,
    triple,
    LLVMGetHostCPUName(), LLVMGetHostCPUFeatures(),
    LLVMCodeGenLevelDefault, LLVMRelocDefault, LLVMCodeModelDefault
  );

  module = LLVMModuleCreateWithName(filename);
  LLVMSetSourceFileName(module, filename, strlen(filename));
  LLVMSetTarget(module, triple);

  LLVMTargetDataRef layout = LLVMCreateTargetDataLayout(machine);
  LLVMSetDataLayout(module, LLVMCopyStringRepOfTargetData(layout));
  LLVMDisposeTargetData(layout);

  builder = LLVMCreateBuilder();

  atexit(LLVMTearDown);
}

/* Global Declarations */

/**
 * Add global variable to default module.
 */
LLVMValueRef DeclareGlobalVariable(char* name, LLVMTypeRef type) {
  return LLVMAddGlobal(module, type, name);
}

/**
 * Add global variable with initial value to default module.
 */
LLVMValueRef DeclareGlobalVariableWithValue(char* name, LLVMTypeRef type, LLVMValueRef value) {
  LLVMValueRef variable = DeclareGlobalVariable(name, type);
  LLVMSetInitializer(variable, value);
  return variable;
}

/**
 * Add global function to default module.
 */
LLVMValueRef DeclareFunction(char* name, LLVMTypeRef type) {
  return LLVMAddFunction(module, name, type);
}

/* Global Variables */

/**
 * Create an integer array with given type and length, and initialized to zero.
 */
LLVMValueRef ConstZeroArray(LLVMTypeRef elementType, int length) {
  LLVMTypeRef type = LLVMArrayType(elementType, length);
  LLVMValueRef zero = LLVMConstInt(elementType, 0, False);
  LLVMValueRef values[length];
  for (int index = 0; index < length; index++) {
    values[index] = zero;
  }
  return LLVMConstArray(elementType, values, length);
}

/* Function Operations */

/**
 * Build call function.
 */
LLVMValueRef call(LLVMTypeRef type, LLVMValueRef fn, int length, LLVMValueRef* parameters) {
  return LLVMBuildCall2(builder, type, fn, parameters, length, "");
}

/**
 * Append a basic block to given function.
 */
LLVMBasicBlockRef Block(LLVMValueRef fn) {
  return LLVMAppendBasicBlock(fn, "");
}

/**
 * Move builder position to end of given block.
 */
void enter(LLVMBasicBlockRef block) {
  LLVMPositionBuilderAtEnd(builder, block);
}

/* Pointer Operations */

/**
 * Build get element pointer.
 */
LLVMValueRef Pointer(LLVMTypeRef type, LLVMValueRef pointer, int length, LLVMValueRef* offset) {
  return LLVMBuildGEP2(builder, type, pointer, offset, length, "");
}

/**
 * Build allocation.
 */
LLVMValueRef alloc(LLVMTypeRef type) {
  return LLVMBuildAlloca(builder, type, "");
}

/**
 * Load value from the pointer.
 */
LLVMValueRef load(LLVMTypeRef type, LLVMValueRef pointer) {
  return LLVMBuildLoad2(builder, type, pointer, "");
}

/**
 * Save value to the pointer.
 */
void store(LLVMValueRef pointer, LLVMValueRef value) {
  LLVMBuildStore(builder, value, pointer);
}

/* Arithmetic Operations */

/**
 * Build plus.
 */
LLVMValueRef add(LLVMValueRef left, LLVMValueRef right) {
  return LLVMBuildAdd(builder, left, right, "");
}

/**
 * Build minus.
 */
LLVMValueRef sub(LLVMValueRef left, LLVMValueRef right) {
  return LLVMBuildSub(builder, left, right, "");
}

/**
 * Build compare.
 */
LLVMValueRef compare(LLVMIntPredicate predicate, LLVMValueRef left, LLVMValueRef right) {
  return LLVMBuildICmp(builder, predicate, left, right, "");
}

/* Control Operations */

/**
 * Build condition branch.
 */
void when(LLVMValueRef condition, LLVMBasicBlockRef then, LLVMBasicBlockRef otherwise) {
  LLVMBuildCondBr(builder, condition, then, otherwise);
}

/**
 * Build branch.
 */
void jumpTo(LLVMBasicBlockRef label) {
  LLVMBuildBr(builder, label);
}

/**
 * Build return a value.
 */
void returnWith(LLVMValueRef value) {
  LLVMBuildRet(builder, value);
}

/**
 * Build return void.
 */
void returnVoid() {
  LLVMBuildRetVoid(builder);
}

/* Conversion Operations */

/**
 * Sign extends type.
 */
LLVMValueRef extend(LLVMValueRef value, LLVMTypeRef type) {
  return LLVMBuildSExt(builder, value, type, "");
}

/**
 * Truncate type.
 */
LLVMValueRef truncate(LLVMValueRef value, LLVMTypeRef type) {
  return LLVMBuildTrunc(builder, value, type, "");
}

/* Object File */

/**
 * Emit object file for the module to given filename.
 */
void EmitObjectFile(char* filename) {
  char* message = NULL;
  if (LLVMTargetMachineEmitToFile(machine, module, filename, LLVMObjectFile, &message) != 0) {
    fprintf(stderr, "LLVM emit object file failed: %s\n", message);
    LLVMDisposeMessage(message);
    exit(EXIT_FAILURE);
  }
}
