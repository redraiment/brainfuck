#ifndef __LINKER_H_
#define __LINKER_H_

#ifdef __cplusplus
extern "C" {
#endif

  void TearDownLinker(void);
  void SetUpLinker(void);
  void Link(char*);

#ifdef __cplusplus
}
#endif

#endif
