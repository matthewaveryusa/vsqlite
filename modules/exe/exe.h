#ifndef EXE_H
#define EXE_H

typedef struct exe{
  int pid;
  char* exe;
  int exe_len;
} exe_t;

void exe_release(void *v);
void exe_print(void *v);

int exevec_get_pid(void* vec, int pid);

#endif
