#ifndef ENV_H
#define ENV_H

#include <stdio.h>

typedef struct env{
  int pid;
  char* name;
  int name_len;
  char* value;
  int value_len;
} env_t;

void env_release(void *v);
void env_print(void *v);

int envvec_get_pid(void* vec, int pid);

#endif
