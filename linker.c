#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "linker.h"

/**
 * Invoke /bin/ld to link the object file to executable ELF file.
 */
void Link(char* object, char* program) {
  char command[
    30 + strlen(LINKER) + strlen(LOADER)
    + strlen(C_RUNTIME_START) + strlen(C_RUNTIME_INIT) + strlen(C_RUNTIME_FINI)
    + strlen(object) + strlen(program)
  ];
  sprintf(command,
          "%s -dynamic-linker %s -lc %s %s %s %s -o %s",
          LINKER, LOADER,
          C_RUNTIME_START, C_RUNTIME_INIT, object, C_RUNTIME_FINI,
          program);
  system(command);
}
