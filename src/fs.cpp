#include <cstdlib>
#include <cstring>

#include <llvm/Support/FileSystem.h>

#include "fs.h"

/**
 * Get a unique temporary file name, not currently exisiting in the filesystem,
 * and allocate spaces to store the name.
 */
char* CreateTemporaryFileName() {
  llvm::SmallString<128> buffer;
  llvm::sys::fs::getPotentiallyUniqueTempFileName("", "", buffer);
  char* path = (char*)calloc(buffer.size() + 1, sizeof(char));
  strcpy(path, buffer.c_str());
  return path;
}

/**
 * Delete the file and free the path spaces.
 */
void DeleteTemporaryFile(char* path) {
  if (path != NULL) {
    llvm::sys::fs::remove(path);
    free(path);
  }
}
