#ifndef __OPTIONS_H_
#define __OPTIONS_H_

/**
 * Executing mode.
 */
typedef enum {
  /* Parse & execute */
  ScriptingMode = 0,
  /* Emit LLVM representation */
  PreprocessMode,
  /* Emit native object */
  CompileMode,
  /* Emit executable file */
  LinkMode
} Mode;

/**
 * Parsed command line options.
 */
typedef struct _Options {
  Mode mode;
  /**
   * Source filename.
   */
  char* source;
  /**
   * Object filename.
   */
  char* object;
  /**
   * Output filename.
   */
  char* output;
} *Options;

Options ParseCommandLineArguments(int, char**);

#endif
