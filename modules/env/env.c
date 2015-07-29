#include <env.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void envvar_init(void *v, const char* blob) {
  envvar_t *envvar = v;

  char* delim=strchr(blob,'=');
  assert(delim != NULL);
  int size = delim-blob;
  envvar->name = (char*) malloc(sizeof(char) * (size+1));
  memcpy(envvar->name,blob,size);
  envvar->name[size] = '\0';
  envvar->name_size = size;

  char* end=strchr(delim,'\0');
  assert(end!= NULL);
  size = end-(delim+1);
  envvar->value = (char*) malloc(sizeof(char) * (size+1));
  memcpy(envvar->value,delim+1,size);
  envvar->value[size] = '\0';
  envvar->value_size = size;
}

void envvar_release(void *v) {
  envvar_t *envvar = v;
  free(envvar->name);
  free(envvar->value);
}

void envvar_print(void *v) {
  envvar_t *envvar = v;
 printf("%s %s\n", envvar->name, envvar->value);
}
