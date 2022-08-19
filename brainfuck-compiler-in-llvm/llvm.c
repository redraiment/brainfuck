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

/* Obtains the default module. */
LLVMModuleRef Module() {
  return module;
}

/* Obtains the builder. */
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
 * Initializes LLVM target machine with given source filename.
 * - creates target machine.
 * - creates module.
 * - creates builder.
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
 * Emits object file for the module to given filename.
 */
void EmitObjectFile(char* filename) {
  char* message = NULL;
  if (LLVMTargetMachineEmitToFile(machine, module, filename, LLVMObjectFile, &message) != 0) {
    fprintf(stderr, "LLVM emit object file failed: %s\n", message);
    LLVMDisposeMessage(message);
    exit(EXIT_FAILURE);
  }
}
