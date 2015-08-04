#include <modules/exe/sqlite3_exe.h>

#include <modules/exe/exe.h>
#include <tools/vector.h>

#include <sqlite3.h>

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>

#include <unistd.h>
#include <linux/limits.h>

int(*exe_sqlite3_setters[])(sqlite3_vtab_cursor*, sqlite3_context*) = {
  exe_sqlite3_pid,
  exe_sqlite3_exe
};


sqlite3_module exe_module = {
  0,
  exe_xCreate,
  exe_xCreate,
  exe_xBestIndex,
  exe_xDestroy,
  exe_xDestroy,
  exe_xOpen,
  exe_xClose,
  exe_xFilter,
  exe_xNext,
  exe_xEof,
  exe_xColumn,
  exe_xRowid,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
};

int exe_xCreate(sqlite3* db, void *pAux, int argc, const char *const*argv, sqlite3_vtab **ppVTab, char **pzErr){
  static char query[] = "CREATE TABLE exe(pid INT, exe TEXT)";
  exe_table_t *table = (exe_table_t*) malloc(sizeof(exe_table_t));
  memset(table,0,sizeof(exe_table_t));
  if(!table) {
    return SQLITE_ERROR;
  }
  table->content = vec_new(sizeof(exe_t),10);
  *ppVTab = (sqlite3_vtab*) table;
  int rc = sqlite3_declare_vtab(db, query);
  if (rc != SQLITE_OK) {
    return rc;
  }
  return SQLITE_OK;
}

int exe_xBestIndex(sqlite3_vtab *pVTab, sqlite3_index_info* index_info){
  return SQLITE_OK;
}

int exe_xDestroy(sqlite3_vtab *pVTab){
  exe_table_t* table = (exe_table_t*) pVTab;
  vec_delete_elems(table->content,exe_release);
  vec_delete(table->content);
  free(pVTab);
  return SQLITE_OK;
}

int exe_xOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  exe_cursor_t *cursor = (exe_cursor_t*) malloc(sizeof(exe_cursor_t));
  if(!cursor) {
    return SQLITE_ERROR;
  }
  cursor->row = 0;
  *ppCursor = (sqlite3_vtab_cursor*) cursor;
  return SQLITE_OK;
}

int exe_xClose(sqlite3_vtab_cursor* cursor){
  free(cursor);
  return SQLITE_OK;
}

int exe_xFilter(sqlite3_vtab_cursor* pCursor, int idxNum, const char *idxStr, int argc, sqlite3_value **argv){
  exe_table_t *table = (exe_table_t*) pCursor->pVtab;
  exe_cursor_t *cursor = (exe_cursor_t*) pCursor;

  //length of a 64 bit int in characters
  int max_pid_len = 20;
  int exe_path_len = sizeof("/proc//exe") + max_pid_len;
  char* exe_path = malloc(exe_path_len);
  if(!exe_path) { return SQLITE_ERROR; }

  struct dirent *ep;
  DIR *dir = opendir ("/proc");
  if (dir == NULL) { return SQLITE_ERROR; }
  while( (ep = readdir(dir)) ) {
    int pid = atoi(ep->d_name);
    if(pid == 0) { continue; }
    assert(strlen(ep->d_name) < exe_path_len);
    exe_path[0] = '\0';
    strcat(exe_path,"/proc/");
    strcat(exe_path,ep->d_name);
    strcat(exe_path,"/exe");
    char temp[PATH_MAX];
    temp[0] = '\0';
    int path_len = readlink(exe_path,temp,sizeof(temp)-1);
    if(path_len < 0) { continue; }
    exe_t *exe = (exe_t*) vec_push_back_uninitialized(table->content,sizeof(exe_t));
    exe->pid = pid;
    exe->exe_len = path_len;
    exe->exe = strdup(temp);
  }
  free(exe_path);
  closedir(dir);
  return SQLITE_OK;
}

int exe_xNext(sqlite3_vtab_cursor* pCursor){
  exe_cursor_t *cursor = (exe_cursor_t*) pCursor;
  ++(cursor->row);
  return SQLITE_OK;
}

int exe_xEof(sqlite3_vtab_cursor* pCursor){
 exe_table_t *table = (exe_table_t*) pCursor->pVtab;
 exe_cursor_t *cursor = (exe_cursor_t*) pCursor;
 return cursor->row >= vec_length(table->content);
}

int exe_sqlite3_pid(sqlite3_vtab_cursor* pCursor, sqlite3_context *ctx) {
  exe_cursor_t *cursor = (exe_cursor_t*) pCursor;
  exe_table_t *table = (exe_table_t*) pCursor->pVtab;
  exe_t* exe = (exe_t*) vec_get(table->content,cursor->row);
  sqlite3_result_int(ctx, exe->pid);
  return SQLITE_OK;
}

int exe_sqlite3_exe(sqlite3_vtab_cursor* pCursor, sqlite3_context *ctx) {
  exe_cursor_t *cursor = (exe_cursor_t*) pCursor;
  exe_table_t *table = (exe_table_t*) pCursor->pVtab;
  exe_t* exe = (exe_t*) vec_get(table->content,cursor->row);
  sqlite3_result_text(ctx, exe->exe, exe->exe_len, SQLITE_STATIC);
  return SQLITE_OK;
}

int exe_xColumn(sqlite3_vtab_cursor* pCursor, sqlite3_context* ctx, int col){
  assert(col <= 1);
  exe_sqlite3_setters[col](pCursor, ctx);
  return SQLITE_OK;
}

int exe_xRowid(sqlite3_vtab_cursor* pCursor, sqlite_int64 *pRowid){
  exe_cursor_t *cursor = (exe_cursor_t*) pCursor;
  *pRowid = cursor->row;
  return SQLITE_OK;
}
