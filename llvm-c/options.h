#ifndef __OPTIONS_H_
#define __OPTIONS_H_

/**
 * Executing mode.
 */
typedef enum {
  /* Emit executable file */
  LinkMode = 0,
  /* Emit native object */
  CompileMode,
  /* Emit LLVM representation */
  RepresentationMode,
  /* Run directly */
  ScriptingMode
} Mode;

/**
 * Parsed command line options.
 */
typedef struct _Options {
  Mode mode;
  /**
   * Singe line command 
   */
  int singleLineCommentEnabled;
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

extern struct _Options options;

void ParseCommandLineArguments(int, char**);

#endif
