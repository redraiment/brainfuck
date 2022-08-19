/**
 * LLVM Wrap
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "llvm.h"

/* default machine. */
LLVMTargetMachineRef machine = NULL;

/* default module. */
LLVMModuleRef module = NULL;

/* default builder. */
LLVMBuilderRef builder = NULL;

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
 * Initialize LLVM target machine with given source file name.
 * - create target machine.
 * - create target data layout.
 * - create module.
 * - create builder.
 */
void LLVMSetUp(char* name) {
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

  module = LLVMModuleCreateWithName(name);
  LLVMSetSourceFileName(module, name, strlen(name));
  LLVMSetTarget(module, triple);

  LLVMTargetDataRef layout = LLVMCreateTargetDataLayout(machine);
  LLVMSetDataLayout(module, LLVMCopyStringRepOfTargetData(layout));
  LLVMDisposeTargetData(layout);

  builder = LLVMCreateBuilder();

  atexit(LLVMTearDown);
}
