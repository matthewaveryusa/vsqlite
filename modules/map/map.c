#include <modules/cmdline/cmdline.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void cmdline_init(void *v,int pid, const char* blob) {
  cmdline_t *cmdline = v;

  cmdline->pid = pid;

  char* delim=strchr(blob,'=');
  assert(delim != NULL);
  int len = delim-blob;
  cmdline->cmdline = (char*) malloc(sizeof(char) * (len+1));
  memcpy(cmdline->cmdline,blob,len);
  cmdline->cmdline[len] = '\0';
  cmdline->cmdline_len = len;
}

void cmdline_release(void *v) {
  cmdline_t *cmdline = v;
  free(cmdline->cmdline);
}

void cmdline_print(void *v) {
  cmdline_t *cmdline = v;
  printf("%i %s\n",cmdline->pid, cmdline->cmdline);
}
