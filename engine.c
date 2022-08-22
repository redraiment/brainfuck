/**
 * Compiler engine: wrap LLVM with single module.
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "engine.h"

/* inner default machine. */
static LLVMTargetMachineRef machine = NULL;

/* inner default module. */
static LLVMModuleRef module = NULL;

/* inner default builder. */
static LLVMBuilderRef builder = NULL;

/**
 * Destroy all LLVM resources.
 */
static void TearDownEngine(void) {
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
 * Initialize LLVM target machine.
 */
void SetUpEngine() {
  atexit(TearDownEngine);

  LLVMLinkInMCJIT();
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
}

/**
 * Set the default module and builder with given module name.
 */
void SetDefaultModule(char* name) {
  module = LLVMModuleCreateWithName(name);
  LLVMSetSourceFileName(module, name, strlen(name));
  LLVMSetTarget(module, LLVMGetDefaultTargetTriple());

  LLVMTargetDataRef layout = LLVMCreateTargetDataLayout(machine);
  LLVMSetDataLayout(module, LLVMCopyStringRepOfTargetData(layout));
  LLVMDisposeTargetData(layout);

  builder = LLVMCreateBuilder();
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
LLVMValueRef CreateZeroInitializer(LLVMTypeRef elementType, int length) {
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
LLVMValueRef CallFunction(LLVMTypeRef type, LLVMValueRef fn, int length, LLVMValueRef* parameters) {
  return LLVMBuildCall2(builder, type, fn, parameters, length, "");
}

/**
 * Append a basic block to given function.
 */
LLVMBasicBlockRef CreateAndAppendBlock(LLVMValueRef fn) {
  return LLVMAppendBasicBlock(fn, "");
}

/**
 * Move builder position to end of given block.
 */
void EnterBlock(LLVMBasicBlockRef block) {
  LLVMPositionBuilderAtEnd(builder, block);
}

/* Pointer Operations */

/**
 * Build get element pointer.
 */
LLVMValueRef GetPointer(LLVMTypeRef type, LLVMValueRef pointer, int length, LLVMValueRef* offset) {
  return LLVMBuildGEP2(builder, type, pointer, offset, length, "");
}

/**
 * Build allocation.
 */
LLVMValueRef Alloc(LLVMTypeRef type) {
  return LLVMBuildAlloca(builder, type, "");
}

/**
 * Load value from the pointer.
 */
LLVMValueRef Load(LLVMTypeRef type, LLVMValueRef pointer) {
  return LLVMBuildLoad2(builder, type, pointer, "");
}

/**
 * Save value to the pointer.
 */
void Store(LLVMValueRef pointer, LLVMValueRef value) {
  LLVMBuildStore(builder, value, pointer);
}

/* Arithmetic Operations */

/**
 * Build plus.
 */
LLVMValueRef Add(LLVMValueRef left, LLVMValueRef right) {
  return LLVMBuildAdd(builder, left, right, "");
}

/**
 * Build minus.
 */
LLVMValueRef Sub(LLVMValueRef left, LLVMValueRef right) {
  return LLVMBuildSub(builder, left, right, "");
}

/**
 * Build compare.
 */
LLVMValueRef Compare(LLVMIntPredicate predicate, LLVMValueRef left, LLVMValueRef right) {
  return LLVMBuildICmp(builder, predicate, left, right, "");
}

/* Control Operations */

/**
 * Build condition branch.
 */
void If(LLVMValueRef condition, LLVMBasicBlockRef then, LLVMBasicBlockRef otherwise) {
  LLVMBuildCondBr(builder, condition, then, otherwise);
}

/**
 * Build branch.
 */
void Goto(LLVMBasicBlockRef label) {
  LLVMBuildBr(builder, label);
}

/**
 * Build return a value.
 */
void Return(LLVMValueRef value) {
  LLVMBuildRet(builder, value);
}

/**
 * Build return void.
 */
void ReturnVoid() {
  LLVMBuildRetVoid(builder);
}

/* Conversion Operations */

/**
 * Sign extends type.
 */
LLVMValueRef ExtendType(LLVMValueRef value, LLVMTypeRef type) {
  return LLVMBuildSExt(builder, value, type, "");
}

/**
 * Truncate type.
 */
LLVMValueRef TruncateType(LLVMValueRef value, LLVMTypeRef type) {
  return LLVMBuildTrunc(builder, value, type, "");
}

/* Output */

/**
 * Emit LLVM IR for the module to file or standard output if filename is NULL.
 */
void EmitIntermediateRepresentation(char* filename) {
  char* ir = LLVMPrintModuleToString(module);
  if (filename == NULL) {
    printf("%s", ir);
  } else {
    FILE* fout = fopen(filename, "w");
    if (fout == NULL) {
      fprintf(stderr, "Open output file %s failed!\n", filename);
      exit(EXIT_FAILURE);
    }
    fprintf(fout, "%s", ir);
    fclose(fout);
  }
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

/**
 * Run machine code of main function with MCJIT execution engine.
 */
void ExecuteMachineCode() {
  LLVMExecutionEngineRef engine = NULL;
  char* message = NULL;
  LLVMCreateJITCompilerForModule(&engine, module, 2, &message);
  if (engine == NULL) {
    fprintf(stderr, "Create JIT compiler failed: %s\n", message);
    LLVMDisposeMessage(message);
    exit(EXIT_FAILURE);
  }
  int (*fn)(void) = (int(*)(void))LLVMGetFunctionAddress(engine, "main");
  fn();
}
