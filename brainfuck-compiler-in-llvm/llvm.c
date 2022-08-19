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

/* Obtain the builder. */
LLVMBuilderRef Builder() {
  return builder;
}

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
