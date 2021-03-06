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
  //for now we just index on an exact match with a pid
  int i;
  for(i = 0; i < index_info->nConstraint; ++i) {
    struct sqlite3_index_constraint *c =  &(index_info->aConstraint[i]);
    if(!c->usable) { continue; }
    if(c->op == SQLITE_INDEX_CONSTRAINT_EQ && c->iColumn == ENV_PID_COLUMN) {
      index_info->aConstraintUsage[i].argvIndex = 1;
      index_info->idxNum = ENV_PID_COLUMN; 
      index_info->estimatedCost = 1;
      return SQLITE_OK;
    }
  }
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

  //exact pid match
  if(idxNum == ENV_PID_COLUMN && argc == 1) {
    int pid = sqlite3_value_int(argv[0]);
    envvec_get_pid(table->content,pid);
    return SQLITE_OK;
  }

  struct dirent *ep;
  DIR *dir = opendir ("/proc");
  if (dir == NULL) { return SQLITE_ERROR; }
  while( (ep = readdir(dir)) ) {
    int pid = atoi(ep->d_name);
    envvec_get_pid(table->content,pid);
  }
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
