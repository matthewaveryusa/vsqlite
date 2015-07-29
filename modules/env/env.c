#include <modules/env/env.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void env_init(void *v,int pid, const char* blob) {
  env_t *env = v;

  env->pid = pid;

  char* delim=strchr(blob,'=');
  assert(delim != NULL);
  int len = delim-blob;
  env->name = (char*) malloc(sizeof(char) * (len+1));
  memcpy(env->name,blob,len);
  env->name[len] = '\0';
  env->name_len = len;

  char* end=strchr(delim,'\0');
  assert(end!= NULL);
  len = end-(delim+1);
  env->value = (char*) malloc(sizeof(char) * (len+1));
  memcpy(env->value,delim+1,len);
  env->value[len] = '\0';
  env->value_len = len;
}

void env_release(void *v) {
  env_t *env = v;
  free(env->name);
  free(env->value);
}

void env_print(void *v) {
  env_t *env = v;
  printf("%i %s %s\n",env->pid, env->name, env->value);
}
