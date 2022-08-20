#include <stdlib.h>
#include <stdio.h>

#include "opts.h"
#include "llvm.h"
#include "lang.h"

int main(int argc, char* argv[]) {
  Options options = CommandLineOptions(argc, argv);

  // initialize
  LLVMSetUp(options->source);
  BrainfuckSetUp();

  FILE* fin = fopen(options->source, "r");
  if (fin == NULL) {
    fprintf(stderr, "Open source file %s failed!\n", options->source);
    exit(EXIT_FAILURE);
  }

  int command = 0;
  while ((command = fgetc(fin)) != EOF) {
    switch (command) {
    case '>':
      move(1);
      break;
    case '<':
      move(-1);
      break;
    case '+':
      update(1);
      break;
    case '-':
      update(-1);
      break;
    case ',':
      input();
      break;
    case '.':
      output();
      break;
    case '[':
      whileNotZero();
      break;
    case ']':
      whileEnd();
      break;
    default:
      /* Ignore Unknown command */
      break;
    }
  }

  fclose(fin);

  // main }
  returnWith(Int32(0));

  // output
  EmitObjectFile(options->output);

  return 0;
}
