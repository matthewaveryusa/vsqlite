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
  //for now we just index on an exact match with a pid
  int i;
  for(i = 0; i < index_info->nConstraint; ++i) {
    struct sqlite3_index_constraint *c =  &(index_info->aConstraint[i]);
    if(!c->usable) { continue; }
    if(c->op == SQLITE_INDEX_CONSTRAINT_EQ && c->iColumn == EXE_PID_COLUMN) {
      index_info->aConstraintUsage[i].argvIndex = 1;
      index_info->idxNum = EXE_PID_COLUMN; 
      index_info->estimatedCost = 1;
      return SQLITE_OK;
    }
  }
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
  
  //exact pid match
  if(idxNum == EXE_PID_COLUMN && argc == 1) {
    int pid = sqlite3_value_int(argv[0]);
    exevec_get_pid(table->content,pid);
    return SQLITE_OK;
  }

  struct dirent *ep;
  DIR *dir = opendir ("/proc");
  if (dir == NULL) { return SQLITE_ERROR; }
  while( (ep = readdir(dir)) ) {
    int pid = atoi(ep->d_name);
    exevec_get_pid(table->content,pid);
  }
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
