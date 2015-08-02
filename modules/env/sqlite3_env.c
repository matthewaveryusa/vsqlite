#include <modules/env/sqlite3_env.h>

#include <modules/env/env.h>
#include <tools/vector.h>

#include <sqlite3.h>

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>

int(*env_sqlite3_setters[])(sqlite3_vtab_cursor*, sqlite3_context*) = {
  env_sqlite3_pid,
  env_sqlite3_name,
  env_sqlite3_value
};


sqlite3_module env_module = {
  0,
  env_xCreate,
  env_xCreate,
  env_xBestIndex,
  env_xDestroy,
  env_xDestroy,
  env_xOpen,
  env_xClose,
  env_xFilter,
  env_xNext,
  env_xEof,
  env_xColumn,
  env_xRowid,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
};

int env_xCreate(sqlite3* db, void *pAux, int argc, const char *const*argv, sqlite3_vtab **ppVTab, char **pzErr){
  static char query[] = "CREATE TABLE env(pid INT, key TEXT, value TEXT)";
  env_table_t *table = (env_table_t*) malloc(sizeof(env_table_t));
  memset(table,0,sizeof(env_table_t));
  if(!table) {
    return SQLITE_ERROR;
  }
  table->content = vec_new(sizeof(env_t),10);
  *ppVTab = (sqlite3_vtab*) table;
  int rc = sqlite3_declare_vtab(db, query);
  if (rc != SQLITE_OK) {
    return rc;
  }
  return SQLITE_OK;
}

int env_xBestIndex(sqlite3_vtab *pVTab, sqlite3_index_info* index_info){
  return SQLITE_OK;
}

int env_xDestroy(sqlite3_vtab *pVTab){
  env_table_t* table = (env_table_t*) pVTab;
  vec_delete_elems(table->content,env_release);
  vec_delete(table->content);
  free(pVTab);
  return SQLITE_OK;
}

int env_xOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  env_cursor_t *cursor = (env_cursor_t*) malloc(sizeof(env_cursor_t));
  if(!cursor) {
    return SQLITE_ERROR;
  }
  cursor->row = 0;
  *ppCursor = (sqlite3_vtab_cursor*) cursor;
  return SQLITE_OK;
}

int env_xClose(sqlite3_vtab_cursor* cursor){
  free(cursor);
  return SQLITE_OK;
}

int env_xFilter(sqlite3_vtab_cursor* pCursor, int idxNum, const char *idxStr, int argc, sqlite3_value **argv){
  env_table_t *table = (env_table_t*) pCursor->pVtab;
  env_cursor_t *cursor = (env_cursor_t*) pCursor;

  //length of a 64 bit int in characters
  int max_pid_len = 20;
  int environ_path_len = sizeof("/proc//environ") + max_pid_len;
  char* environ_path = malloc(environ_path_len);
  if(!environ_path) { return SQLITE_ERROR; }

  struct dirent *ep;
  DIR *dir = opendir ("/proc");
  if (dir == NULL) { return SQLITE_ERROR; }
  while( (ep = readdir(dir)) ) {
    int pid = atoi(ep->d_name);
    if(pid == 0) { continue; }
    assert(strlen(ep->d_name) < environ_path_len);
    environ_path[0] = '\0';
    strcat(environ_path,"/proc/");
    strcat(environ_path,ep->d_name);
    strcat(environ_path,"/environ");
    FILE *fp = fopen(environ_path,"r");
    if(!fp) { continue; }
    typedef enum state { KEY, VALUE} state;
    char c;
    state s = KEY;
    void * temp = vec_new(1,10);
    char *t;
    env_t *env;
    while ( (c = fgetc(fp)) != EOF) {
      switch (s) {
        case KEY:
          if(c == '=') {
            env = (env_t*) vec_push_back_uninitialized(table->content,sizeof(env_t));
            env->pid = pid;
            env->name_len = vec_length(temp);
            t = vec_push_back_uninitialized(temp,1);
            *t = '\0';
            env->name = (char*) vec_move_and_delete(temp);
            s = VALUE;
            temp = vec_new(1,10);
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
  }
  free(environ_path);
  closedir(dir);
  return SQLITE_OK;
}

int env_xNext(sqlite3_vtab_cursor* pCursor){
  env_cursor_t *cursor = (env_cursor_t*) pCursor;
  ++(cursor->row);
  return SQLITE_OK;
}

int env_xEof(sqlite3_vtab_cursor* pCursor){
 env_table_t *table = (env_table_t*) pCursor->pVtab;
 env_cursor_t *cursor = (env_cursor_t*) pCursor;
 return cursor->row >= vec_length(table->content);
}

int env_sqlite3_pid(sqlite3_vtab_cursor* pCursor, sqlite3_context *ctx) {
  env_cursor_t *cursor = (env_cursor_t*) pCursor;
  env_table_t *table = (env_table_t*) pCursor->pVtab;
  env_t* env = (env_t*) vec_get(table->content,cursor->row);
  sqlite3_result_int(ctx, env->pid);
  return SQLITE_OK;
}

int env_sqlite3_name(sqlite3_vtab_cursor* pCursor, sqlite3_context *ctx) {
  env_cursor_t *cursor = (env_cursor_t*) pCursor;
  env_table_t *table = (env_table_t*) pCursor->pVtab;
  env_t* env = (env_t*) vec_get(table->content,cursor->row);
  sqlite3_result_text(ctx, env->name, env->name_len, SQLITE_STATIC);
  return SQLITE_OK;
}

int env_sqlite3_value(sqlite3_vtab_cursor* pCursor, sqlite3_context *ctx) {
  env_cursor_t *cursor = (env_cursor_t*) pCursor;
  env_table_t *table = (env_table_t *) pCursor->pVtab;
  env_t* env = (env_t*) vec_get(table->content,cursor->row);
  sqlite3_result_text(ctx, env->value, env->value_len, SQLITE_STATIC);
  return SQLITE_OK;
}

int env_xColumn(sqlite3_vtab_cursor* pCursor, sqlite3_context* ctx, int col){
  assert(col <= 2);
  env_sqlite3_setters[col](pCursor, ctx);
  return SQLITE_OK;
}

int env_xRowid(sqlite3_vtab_cursor* pCursor, sqlite_int64 *pRowid){
  env_cursor_t *cursor = (env_cursor_t*) pCursor;
  *pRowid = cursor->row;
  return SQLITE_OK;
}
