#include <modules/map/map.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <tools/vector.h>

void map_release(void *v) {
  map_t *map = v;
  free(map->map);
}

void map_print(void *v) {
  map_t *map = v;
  printf("%i %s\n",map->pid, map->map);
}

int mapvec_get_pid(void* vec, int pid) {
  //length of a 64 bit int in characters
  int count = 0;
  if(pid == 0) { return count; }
  static const int max_pid_len = 20;
  char map_path[sizeof("/proc//map") + 20];
  snprintf(map_path, sizeof(map_path),"/proc/%i/map",pid);
  FILE *fp = fopen(map_path,"r");
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
    map_t *map = (map_t*) vec_push_back_uninitialized(vec,sizeof(map_t));
    map->pid = pid;
    map->map_len = vec_length(temp);
    map->map = vec_move_and_delete(temp);
    ++count;
  }
  fclose(fp);
  return count;
}
