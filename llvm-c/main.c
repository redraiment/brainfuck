#include <stdlib.h>
#include <stdio.h>

#include "options.h"
#include "engine.h"
#include "compiler.h"

int main(int argc, char* argv[]) {
  SetUpCompiler();

  Options options = ParseCommandLineArguments(argc, argv);
  Compile(options->source);
  switch (options->mode) {
  case PreprocessMode:
    EmitIntermediateRepresentation(options->output);
    break;
  case CompileMode:
    EmitObjectFile(options->output);
    break;
  case ScriptingMode:
    ExecuteMachineCode();
    break;
  }

  return 0;
}
