#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include <dirent.h>
#include <unistd.h>

#include <lld/Common/Driver.h>
#include <lld/Common/LLVM.h>

#include "linker.h"

static const char* MUSL_LIBRARY_FOLDERS[] = {
  // debian/ubuntu
  "/usr/lib/x86_64-linux-musl",
  "/usr/lib/i386-linux-musl",
  // centos/rhel
  "/usr/x86_64-linux-musl/lib64/",
  "/usr/i386-linux-musl/lib/",
  // archlinux
  "/usr/lib/musl/lib",
  // alpine
  "/usr/lib/"
};
static const char* MUSL_LIBRARY_FILES[] = {
  "Scrt1.o",
  "crti.o",
  "crtn.o",
  "libc.a"
};

static int MUSL_LIBRARY_FOLDER_SIZE = sizeof(MUSL_LIBRARY_FOLDERS) / sizeof(char*);
static int MUSL_LIBRARY_FILES_SIZE = sizeof(MUSL_LIBRARY_FILES) / sizeof(char*);

static char const * MUSL_HOME = NULL;
static char** MUSL_LIBRARY_PATHS = NULL;

/**
 * Check if the specified folder exists.
 */
static bool FolderExists(const char* path) {
  DIR* dir = opendir(path);
  if (dir != NULL) {
    closedir(dir);
    return true;
  }
  return false;
}

/**
 * Check if the specified file exists.
 */
static bool FileExists(const char* path) {
  return access(path, F_OK) == 0;
}

/**
 * Shutdown linker and clear memory.
 */
void TearDownLinker() {
  if (MUSL_LIBRARY_PATHS != NULL) {
    for (int index = 0; index < MUSL_LIBRARY_FILES_SIZE; index++) {
      free(MUSL_LIBRARY_PATHS[index]);
    }
    free(MUSL_LIBRARY_PATHS);
    MUSL_LIBRARY_PATHS = NULL;
  }
}

/**
 * Search and initialize musl library path.
 */
void SetUpLinker() {
  MUSL_HOME = getenv("MUSL_HOME");
  if (MUSL_HOME == NULL) {
    for (int index = 0; index < MUSL_LIBRARY_FOLDER_SIZE; index++) {
      if (FolderExists(MUSL_LIBRARY_FOLDERS[index])) {
        MUSL_HOME = MUSL_LIBRARY_FOLDERS[index];
        break;
      }
    }
  } else if (!FolderExists(MUSL_HOME)) {
    MUSL_HOME = NULL;
  }

  if (MUSL_HOME == NULL) {
    fprintf(stderr, "Cannot found musl library, specify it via environment variable `MUSL_HOME'.\n");
    exit(EXIT_FAILURE);
  }

  int length = strlen(MUSL_HOME);
  MUSL_LIBRARY_PATHS = (char**)calloc(MUSL_LIBRARY_FILES_SIZE, sizeof(char*));
  for (int index = 0; index < MUSL_LIBRARY_FILES_SIZE; index++) {
    const char* file = MUSL_LIBRARY_FILES[index];
    MUSL_LIBRARY_PATHS[index] = (char*)calloc(length + strlen(file) + 2, sizeof(char));
    sprintf(MUSL_LIBRARY_PATHS[index], "%s/%s", MUSL_HOME, file);
    if (!FileExists(MUSL_LIBRARY_PATHS[index])) {
      fprintf(stderr, "Cannot found %s under %s.\n", file, MUSL_HOME);
      exit(EXIT_FAILURE);
    }
  }
}

/**
 * Link the object file to executable ELF file.
 */
void Link(char* object, char* program) {
  std::vector<const char *> args;

  args.push_back("ld.lld");
  args.push_back("-static");
  args.push_back("-o");
  args.push_back(program);
  for (int index = 0; index < MUSL_LIBRARY_FILES_SIZE; index++) {
    args.push_back(MUSL_LIBRARY_PATHS[index]);
  }
  args.push_back(object);

  lld::elf::link(args, llvm::outs(), llvm::errs(), false, false);
}
