#ifndef __FS_H_
#define __FS_H_

#ifdef __cplusplus
extern "C" {
#endif

  char* CreateTemporaryFileName();
  void DeleteTemporaryFile(char*);

  #ifdef __cplusplus
}
#endif

#endif
