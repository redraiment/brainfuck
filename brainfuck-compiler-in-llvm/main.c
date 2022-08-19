#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

  // main {
  update(10);

  // while {
  whileNotZero(); {
    move(1);
    update(7);

    move(1);
    update(10);
  
    move(1);
    update(3);
  
    move(1);
    update(1);
  
    move(-4);
    update(-1);
  }
  // while }
  whileEnd();

  // H
  move(1);
  update(2);
  output();

  // e
  move(1);
  update(1);
  output();

  // l
  update(7);
  output();

  // l
  output();

  // o
  update(3);
  output();

  // \space
  move(1);
  update(2);
  output();

  // W
  move(-2);
  update(15);
  output();

  // o
  move(1);
  output();

  // r
  update(3);
  output();

  // l
  update(-6);
  output();

  // d
  update(-8);
  output();

  // !
  move(1);
  update(1);
  output();

  // \newline
  move(1);
  output();

  // main }
  returnWith(Int32(0));

  // output
  EmitObjectFile(object);

  return 0;
}
