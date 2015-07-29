#ifndef ENV_H
#define ENV_H

typedef struct env{
  int pid;
  char* name;
  int name_len;
  char* value;
  int value_len;
} env_t;

void env_init(void *v, int pid, const char* blob);
void env_release(void *v);
void env_print(void *v);

#endif
