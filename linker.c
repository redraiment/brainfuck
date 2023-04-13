#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "linker.h"

static char* library_search_path[] = {
  "/usr/lib/x86_64-linux-musl",
  "/usr/lib/i386-linux-musl",
  "/lib",
  "/lib/i386-linux-gnu",
  "/lib/x86_64-linux-gnu",
  "/lib64",
  "/usr/lib",
  "/usr/lib/i386-linux-gnu",
  "/usr/lib/x86_64-linux-gnu",
  "/usr/lib64"
};
static int library_search_path_size = 10;

static char* binary_search_path[] = {
  "/bin",
  "/sbin",
  "/usr/bin",
  "/usr/sbin",
};
static int binary_search_path_size = 4;

static char* search_file(char* path[], int size, char* name, char* output) {
  for (int index = 0 ; index < size; index++) {
    sprintf(output, "%s/%s", path[index], name);
    if (access(output, F_OK) == 0) {
      return output;
    }
  }
  output[0] = 0;
  return NULL;
}

static char LINKER[128];
static char C_RUNTIME_START[128];
static char C_RUNTIME_INIT[128];
static char C_RUNTIME_FINI[128];
static char C_LIBRARY_PATH[128];

static char* search_library(char* name, char* output) {
  return search_file(library_search_path, library_search_path_size, name, output);
}

static char* search_binary(char* name, char* output) {
  return search_file(binary_search_path, binary_search_path_size, name, output);
}

/**
 * Invoke /bin/ld to link the object file to executable ELF file.
 */
void Link(char* object, char* program) {
  search_binary("ld", LINKER);
  search_library("Scrt1.o", C_RUNTIME_START);
  search_library("crti.o", C_RUNTIME_INIT);
  search_library("crtn.o", C_RUNTIME_FINI);
  search_library("libc.a", C_LIBRARY_PATH);
  C_LIBRARY_PATH[strlen(C_LIBRARY_PATH) - 7] = 0;
  char command[
               30 + strlen(LINKER) + strlen(C_LIBRARY_PATH)
               + strlen(C_RUNTIME_START) + strlen(C_RUNTIME_INIT) + strlen(C_RUNTIME_FINI)
               + strlen(object) + strlen(program)
               ];
  sprintf(command,
          "%s -static -L%s %s %s %s -lc %s -o %s",
          LINKER, C_LIBRARY_PATH,
          C_RUNTIME_START, C_RUNTIME_INIT, object, C_RUNTIME_FINI,
          program);
  system(command);
}
