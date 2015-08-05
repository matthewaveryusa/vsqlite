#include <modules/cmdline/sqlite3_cmdline.h>

#include <modules/cmdline/cmdline.h>
#include <tools/vector.h>

#include <sqlite3.h>

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>

int(*cmdline_sqlite3_setters[])(sqlite3_vtab_cursor*, sqlite3_context*) = {
  cmdline_sqlite3_pid,
  cmdline_sqlite3_cmdline
};


sqlite3_module cmdline_module = {
  0,
  cmdline_xCreate,
  cmdline_xCreate,
  cmdline_xBestIndex,
  cmdline_xDestroy,
  cmdline_xDestroy,
  cmdline_xOpen,
  cmdline_xClose,
  cmdline_xFilter,
  cmdline_xNext,
  cmdline_xEof,
  cmdline_xColumn,
  cmdline_xRowid,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
};

int cmdline_xCreate(sqlite3* db, void *pAux, int argc, const char *const*argv, sqlite3_vtab **ppVTab, char **pzErr){
  static char query[] = "CREATE TABLE cmdline(pid INT, cmdline TEXT)";
  cmdline_table_t *table = (cmdline_table_t*) malloc(sizeof(cmdline_table_t));
  memset(table,0,sizeof(cmdline_table_t));
  if(!table) {
    return SQLITE_ERROR;
  }
  table->content = vec_new(sizeof(cmdline_t),10);
  *ppVTab = (sqlite3_vtab*) table;
  int rc = sqlite3_declare_vtab(db, query);
  if (rc != SQLITE_OK) {
    return rc;
  }
  return SQLITE_OK;
}

int cmdline_xBestIndex(sqlite3_vtab *pVTab, sqlite3_index_info* index_info){
  //for now we just index on an exact match with a pid
  int i;
  for(i = 0; i < index_info->nConstraint; ++i) {
    struct sqlite3_index_constraint *c =  &(index_info->aConstraint[i]);
    if(!c->usable) { continue; }
    if(c->op == SQLITE_INDEX_CONSTRAINT_EQ && c->iColumn == CMDLINE_PID_COLUMN) {
      index_info->aConstraintUsage[i].argvIndex = 1;
      index_info->idxNum = CMDLINE_PID_COLUMN; 
      index_info->estimatedCost = 1;
      return SQLITE_OK;
    }
  }
  return SQLITE_OK;
}

int cmdline_xDestroy(sqlite3_vtab *pVTab){
  cmdline_table_t* table = (cmdline_table_t*) pVTab;
  vec_delete_elems(table->content,cmdline_release);
  vec_delete(table->content);
  free(pVTab);
  return SQLITE_OK;
}

int cmdline_xOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  cmdline_cursor_t *cursor = (cmdline_cursor_t*) malloc(sizeof(cmdline_cursor_t));
  if(!cursor) {
    return SQLITE_ERROR;
  }
  cursor->row = 0;
  *ppCursor = (sqlite3_vtab_cursor*) cursor;
  return SQLITE_OK;
}

int cmdline_xClose(sqlite3_vtab_cursor* cursor){
  free(cursor);
  return SQLITE_OK;
}

int cmdline_xFilter(sqlite3_vtab_cursor* pCursor, int idxNum, const char *idxStr, int argc, sqlite3_value **argv){
  cmdline_table_t *table = (cmdline_table_t*) pCursor->pVtab;
  cmdline_cursor_t *cursor = (cmdline_cursor_t*) pCursor;
  
  //exact pid match
  if(idxNum == CMDLINE_PID_COLUMN && argc == 1) {
    int pid = sqlite3_value_int(argv[0]);
    cmdlinevec_get_pid(table->content,pid);
    return SQLITE_OK;
  }

  struct dirent *ep;
  DIR *dir = opendir ("/proc");
  if (dir == NULL) { return SQLITE_ERROR; }
  while( (ep = readdir(dir)) ) {
    int pid = atoi(ep->d_name);
    cmdlinevec_get_pid(table->content,pid);
  }
  closedir(dir);
  return SQLITE_OK;
}

int cmdline_xNext(sqlite3_vtab_cursor* pCursor){
  cmdline_cursor_t *cursor = (cmdline_cursor_t*) pCursor;
  ++(cursor->row);
  return SQLITE_OK;
}

int cmdline_xEof(sqlite3_vtab_cursor* pCursor){
 cmdline_table_t *table = (cmdline_table_t*) pCursor->pVtab;
 cmdline_cursor_t *cursor = (cmdline_cursor_t*) pCursor;
 return cursor->row >= vec_length(table->content);
}

int cmdline_sqlite3_pid(sqlite3_vtab_cursor* pCursor, sqlite3_context *ctx) {
  cmdline_cursor_t *cursor = (cmdline_cursor_t*) pCursor;
  cmdline_table_t *table = (cmdline_table_t*) pCursor->pVtab;
  cmdline_t* cmdline = (cmdline_t*) vec_get(table->content,cursor->row);
  sqlite3_result_int(ctx, cmdline->pid);
  return SQLITE_OK;
}

int cmdline_sqlite3_cmdline(sqlite3_vtab_cursor* pCursor, sqlite3_context *ctx) {
  cmdline_cursor_t *cursor = (cmdline_cursor_t*) pCursor;
  cmdline_table_t *table = (cmdline_table_t*) pCursor->pVtab;
  cmdline_t* cmdline = (cmdline_t*) vec_get(table->content,cursor->row);
  sqlite3_result_text(ctx, cmdline->cmdline, cmdline->cmdline_len, SQLITE_STATIC);
  return SQLITE_OK;
}

int cmdline_xColumn(sqlite3_vtab_cursor* pCursor, sqlite3_context* ctx, int col){
  assert(col <= 1);
  cmdline_sqlite3_setters[col](pCursor, ctx);
  return SQLITE_OK;
}

int cmdline_xRowid(sqlite3_vtab_cursor* pCursor, sqlite_int64 *pRowid){
  cmdline_cursor_t *cursor = (cmdline_cursor_t*) pCursor;
  *pRowid = cursor->row;
  return SQLITE_OK;
}
