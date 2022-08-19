#include <stdlib.h>
#include <stdio.h>

#include "llvm.h"
#include "lang.h"

int main(int argc, char* argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <source-file> <object-file>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  char* source = argv[1]; /* source filename */
  char* object = argv[2]; /* object filename */

  // initialize
  LLVMSetUp(source);
  BrainfuckSetUp();

  FILE* fin = fopen(source, "r");
  if (fin == NULL) {
    fprintf(stderr, "Open source file %s failed!\n", source);
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
  EmitObjectFile(object);

  return 0;
}
