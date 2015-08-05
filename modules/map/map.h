#ifndef MAP_H
#define MAP_H

typedef struct map{
  int pid;
  char* map;
  int map_len;
} map_t;

void map_release(void *v);
void map_print(void *v);

int mapvec_get_pid(void* vec, int pid);

#endif
