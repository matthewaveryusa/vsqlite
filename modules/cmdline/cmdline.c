#include <modules/cmdline/cmdline.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void cmdline_release(void *v) {
  cmdline_t *cmdline = v;
  free(cmdline->cmdline);
}

void cmdline_print(void *v) {
  cmdline_t *cmdline = v;
  printf("%i %s\n",cmdline->pid, cmdline->cmdline);
}
