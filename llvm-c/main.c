#include <stdlib.h>
#include <stdio.h>

#include "options.h"
#include "engine.h"
#include "compiler.h"

int main(int argc, char* argv[]) {
  CompilerSetUp();

  Options options = CommandLineOptions(argc, argv);
  Compile(options->source);
  switch (options->mode) {
  case PreprocessMode:
    EmitIntermediateRepresentation(options->output);
    break;
  case CompileMode:
    EmitObjectFile(options->output);
  case ScriptingMode:
    break;
  }

  return 0;
}
