#ifndef ENVVAR_H
#define ENVVAR_H

typedef struct envvar{
  char* name;
  int name_size;
  char* value;
  int value_size;
} envvar_t;

void envvar_init(void *v, const char* blob);
void envvar_release(void *v);
void envvar_print(void *v);

#endif
