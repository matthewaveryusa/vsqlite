#include <sqlite_env.h>

#include <envvar.h>
#include <vector.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <assert.h>

extern char **environ;
int(*env_sqlite3_setters[])(sqlite3_vtab_cursor*, sqlite3_context*) = {
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
  static char query[] = "CREATE TABLE env(key TEXT, value TEXT)";
  env_table_t *table = (env_table_t*) malloc(sizeof(env_table_t));
  if(!table) {
    return SQLITE_ERROR;
  }
  table->content = vec_new(sizeof(envvar_t),10);
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
  vec_delete_elems(table->content,envvar_release);
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
 char *s = *environ;
 int i;
 for(i = 0;s; i++) {
   void *envvar = vec_push_back_uninitialized(table->content,sizeof(envvar_t));
   envvar_init(envvar,s);
   s = *(environ+i+1);
 }
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

int env_sqlite3_name(sqlite3_vtab_cursor* pCursor, sqlite3_context *ctx) {
  env_cursor_t *cursor = (env_cursor_t*) pCursor;
  env_table_t *table = (env_table_t*) pCursor->pVtab;
  envvar_t* envvar = (envvar_t*) vec_get(table->content,cursor->row);
  sqlite3_result_text(ctx, envvar->name, envvar->name_size, SQLITE_STATIC);
  return SQLITE_OK;
}

int env_sqlite3_value(sqlite3_vtab_cursor* pCursor, sqlite3_context *ctx) {
  env_cursor_t *cursor = (env_cursor_t*) pCursor;
  env_table_t *table = (env_table_t *) pCursor->pVtab;
  envvar_t* envvar = (envvar_t*) vec_get(table->content,cursor->row);
  sqlite3_result_text(ctx, envvar->value, envvar->value_size, SQLITE_STATIC);
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
