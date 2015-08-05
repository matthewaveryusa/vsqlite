#include <modules/cmdline/cmdline.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <tools/vector.h>

void cmdline_release(void *v) {
  cmdline_t *cmdline = v;
  free(cmdline->cmdline);
}

void cmdline_print(void *v) {
  cmdline_t *cmdline = v;
  printf("%i %s\n",cmdline->pid, cmdline->cmdline);
}

int cmdlinevec_get_pid(void* vec, int pid) {
  //length of a 64 bit int in characters
  int count = 0;
  if(pid == 0) { return count; }
  static const int max_pid_len = 20;
  char cmdline_path[sizeof("/proc//cmdline") + 20];
  snprintf(cmdline_path, sizeof(cmdline_path),"/proc/%i/cmdline",pid);
  FILE *fp = fopen(cmdline_path,"r");
  if(!fp) { return count; }
  char c;
  char *t;
  void * temp = vec_new(1,10);
  int null_count = 0;
  while ( (c = fgetc(fp)) != EOF) {
    if(c == '\0') {
      ++null_count;
      continue;
    }
    if(null_count != 0) { 
      if(c == '\0') {
        assert(null_count < 2);
        ++null_count;
        continue;
      } else {
        assert(null_count == 1);
        t = vec_push_back_uninitialized(temp,1);
        *t = ' ';
        null_count = 0;
      }
    }
    t = vec_push_back_uninitialized(temp,1);
    *t = c;
  }
  if(vec_length(temp) != 0) {
    cmdline_t *cmdline = (cmdline_t*) vec_push_back_uninitialized(vec,sizeof(cmdline_t));
    cmdline->pid = pid;
    cmdline->cmdline_len = vec_length(temp);
    cmdline->cmdline = vec_move_and_delete(temp);
    ++count;
  }
  fclose(fp);
  return count;
}
