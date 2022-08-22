#include <stdlib.h>
#include <stdio.h>

#include "options.h"
#include "engine.h"
#include "compiler.h"
#include "linker.h"

int main(int argc, char* argv[]) {
  SetUpCompiler();

  Options options = ParseCommandLineArguments(argc, argv);
  Compile(options->source);
  switch (options->mode) {
  case ScriptingMode:
    ExecuteMachineCode();
    break;
  case CompileMode:
    EmitObjectFile(options->output);
    break;
  case RepresentationMode:
    EmitIntermediateRepresentation(options->output);
    break;
  default:
    EmitObjectFile(options->object);
    Link(options->object, options->output);
    break;
  }

  return 0;
}
