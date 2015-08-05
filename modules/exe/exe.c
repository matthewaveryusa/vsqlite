#include <modules/exe/exe.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/limits.h>
#include <tools/vector.h>

void exe_release(void *v) {
  exe_t *exe = v;
  free(exe->exe);
}

void exe_print(void *v) {
  exe_t *exe = v;
  printf("%i %s\n",exe->pid, exe->exe);
}

int exevec_get_pid(void* vec, int pid) {
    int count = 0;
    if(pid == 0) { return count; }
    static const int max_pid_len = 20;
    char exe_path[sizeof("/proc//exe") + 20];
    snprintf(exe_path, sizeof(exe_path),"/proc/%i/exe",pid);
    char temp[PATH_MAX];
    temp[0] = '\0';
    int path_len = readlink(exe_path,temp,sizeof(temp)-1);
    if(path_len < 0) { return count; }
    exe_t *exe = (exe_t*) vec_push_back_uninitialized(vec,sizeof(exe_t));
    exe->pid = pid;
    exe->exe_len = path_len;
    exe->exe = strdup(temp);
    ++count;
    return count;
}
