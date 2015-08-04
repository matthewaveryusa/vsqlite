#ifndef CMDLINE_H
#define CMDLINE_H

typedef struct cmdline{
  int pid;
  char* cmdline;
  int cmdline_len;
} cmdline_t;

void cmdline_init(void *v, int pid, const char* blob);
void cmdline_release(void *v);
void cmdline_print(void *v);

#endif
