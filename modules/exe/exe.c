#include <modules/exe/exe.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void exe_release(void *v) {
  exe_t *exe = v;
  free(exe->exe);
}

void exe_print(void *v) {
  exe_t *exe = v;
  printf("%i %s\n",exe->pid, exe->exe);
}
