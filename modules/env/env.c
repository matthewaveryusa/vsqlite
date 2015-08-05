#include <modules/env/env.h>
#include <tools/vector.h>

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void env_release(void *v) {
  env_t *env = v;
  free(env->name);
  free(env->value);
}

int envvec_get_pid(void* vec, int pid) {
  int count = 0;
  if(pid == 0) { return count; }
  static const int max_pid_len = 20;
  char environ_path[sizeof("/proc//environ") + 20];
  snprintf(environ_path, sizeof(environ_path),"/proc/%i/environ",pid);
  FILE *fp = fopen(environ_path,"r");
  if(!fp) { return count; }
  env_t *env;
  typedef enum state { KEY, VALUE} state;
  void * temp = vec_new(1,10);
  char *t;
  char c;
  state s = KEY;
  while ( (c = fgetc(fp)) != EOF) {
    switch (s) {
      case KEY:
        if(c == '=') {
          env = (env_t*) vec_push_back_uninitialized(vec,sizeof(env_t));
          env->name_len = vec_length(temp);
          env->pid = pid;
          t = vec_push_back_uninitialized(temp,1);
          *t = '\0';
          env->name = (char*) vec_move_and_delete(temp);
          s = VALUE;
          temp = vec_new(1,10);
          ++count;
        } else {
          t = vec_push_back_uninitialized(temp,1);
          *t = c;
        }
        break;
      case VALUE:
        if(c == '\0') {
          env->value_len = vec_length(temp);
          t = vec_push_back_uninitialized(temp,1);
          *t = '\0';
          env->value = (char*) vec_move_and_delete(temp);
          s = KEY;
          temp = vec_new(1,10);
        } else {
          t = vec_push_back_uninitialized(temp,1);
          *t = c;
        }
        break;
    }
  }
  vec_delete(temp);
  fclose(fp);
  return count;
}

void env_print(void *v) {
  env_t *env = v;
  printf("%i %s %s\n",env->pid, env->name, env->value);
}
