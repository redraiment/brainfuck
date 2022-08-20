#ifndef __OPTIONS_H_
#define __OPTIONS_H_

/**
 * Executing mode.
 */
typedef enum {
  /* Parse & execute */
  ScriptingMode = 0,
  /* Emit native object */
  CompileMode,
  /* Emit LLVM representation */
  PreprocessMode
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
   * Output filename.
   */
  char* output;
} *Options;

Options CommandLineOptions(int, char**);

#endif
