#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include <dirent.h>
#include <unistd.h>

#include <lld/Common/Driver.h>
#include <lld/Common/LLVM.h>

#include "crt.h"
#include "engine.h"
#include "fs.h"
#include "linker.h"

static char* crt1 = NULL;
static char* crti = NULL;
static char* crtn = NULL;
static char* libc = NULL;

/**
* Shutdown linker and clear memory. // // 
*/
void TearDownLinker() {
  DeleteTemporaryFile(crt1);
  DeleteTemporaryFile(crti);
  DeleteTemporaryFile(crtn);
  DeleteTemporaryFile(libc);
}

static char* SaveToTemporaryFile(unsigned char* content, unsigned int length) {
  char* path = CreateTemporaryFileName();
  FILE* file = fopen(path, "wb");
  if (file != NULL) {
    fwrite(content, sizeof(unsigned char), length, file);
    fclose(file);
  } else {
    fprintf(stderr, "Generate C Runtime Library files failed!\n");
    exit(EXIT_FAILURE);
  }
  return path;
}

/**
 * Search and initialize musl library path.
 */
void SetUpLinker() {
  crt1 = SaveToTemporaryFile(SCRT1_O, SCRT1_O_LEN);
  crti = SaveToTemporaryFile(CRTI_O, CRTI_O_LEN);
  crtn = SaveToTemporaryFile(CRTN_O, CRTN_O_LEN);
  libc = SaveToTemporaryFile(LIBC_A, LIBC_A_LEN);
}

/**
 * Link the object file to executable ELF file.
 */
void Link(char* program) {
  char* object = CreateTemporaryFileName();
  EmitObjectFile(object);

  std::vector<const char *> args;
  args.push_back("ld.lld");
  args.push_back("-static");
  args.push_back("-o");
  args.push_back(program);
  args.push_back(crt1);
  args.push_back(crti);
  args.push_back(object);
  args.push_back(libc);
  args.push_back(crtn);

  lld::elf::link(args, llvm::outs(), llvm::errs(), false, false);

  DeleteTemporaryFile(object);
}
