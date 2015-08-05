#include <modules/map/sqlite3_map.h>

#include <modules/map/map.h>
#include <tools/vector.h>

#include <sqlite3.h>

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>

int(*map_sqlite3_setters[])(sqlite3_vtab_cursor*, sqlite3_context*) = {
  map_sqlite3_pid,
  map_sqlite3_map
};


sqlite3_module map_module = {
  0,
  map_xCreate,
  map_xCreate,
  map_xBestIndex,
  map_xDestroy,
  map_xDestroy,
  map_xOpen,
  map_xClose,
  map_xFilter,
  map_xNext,
  map_xEof,
  map_xColumn,
  map_xRowid,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
};

int map_xCreate(sqlite3* db, void *pAux, int argc, const char *const*argv, sqlite3_vtab **ppVTab, char **pzErr){
  static char query[] = "CREATE TABLE map(pid INT, map TEXT)";
  map_table_t *table = (map_table_t*) malloc(sizeof(map_table_t));
  memset(table,0,sizeof(map_table_t));
  if(!table) {
    return SQLITE_ERROR;
  }
  table->content = vec_new(sizeof(map_t),10);
  *ppVTab = (sqlite3_vtab*) table;
  int rc = sqlite3_declare_vtab(db, query);
  if (rc != SQLITE_OK) {
    return rc;
  }
  return SQLITE_OK;
}

int map_xBestIndex(sqlite3_vtab *pVTab, sqlite3_index_info* index_info){
  //for now we just index on an exact match with a pid
  int i;
  for(i = 0; i < index_info->nConstraint; ++i) {
    struct sqlite3_index_constraint *c =  &(index_info->aConstraint[i]);
    if(!c->usable) { continue; }
    if(c->op == SQLITE_INDEX_CONSTRAINT_EQ && c->iColumn == MAP_PID_COLUMN) {
      index_info->aConstraintUsage[i].argvIndex = 1;
      index_info->idxNum = MAP_PID_COLUMN; 
      index_info->estimatedCost = 1;
      return SQLITE_OK;
    }
  }
  return SQLITE_OK;
}

int map_xDestroy(sqlite3_vtab *pVTab){
  map_table_t* table = (map_table_t*) pVTab;
  vec_delete_elems(table->content,map_release);
  vec_delete(table->content);
  free(pVTab);
  return SQLITE_OK;
}

int map_xOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  map_cursor_t *cursor = (map_cursor_t*) malloc(sizeof(map_cursor_t));
  if(!cursor) {
    return SQLITE_ERROR;
  }
  cursor->row = 0;
  *ppCursor = (sqlite3_vtab_cursor*) cursor;
  return SQLITE_OK;
}

int map_xClose(sqlite3_vtab_cursor* cursor){
  free(cursor);
  return SQLITE_OK;
}

int map_xFilter(sqlite3_vtab_cursor* pCursor, int idxNum, const char *idxStr, int argc, sqlite3_value **argv){
  map_table_t *table = (map_table_t*) pCursor->pVtab;
  map_cursor_t *cursor = (map_cursor_t*) pCursor;
  
  //exact pid match
  if(idxNum == MAP_PID_COLUMN && argc == 1) {
    int pid = sqlite3_value_int(argv[0]);
    mapvec_get_pid(table->content,pid);
    return SQLITE_OK;
  }

  struct dirent *ep;
  DIR *dir = opendir ("/proc");
  if (dir == NULL) { return SQLITE_ERROR; }
  while( (ep = readdir(dir)) ) {
    int pid = atoi(ep->d_name);
    mapvec_get_pid(table->content,pid);
  }
  closedir(dir);
  return SQLITE_OK;
}

int map_xNext(sqlite3_vtab_cursor* pCursor){
  map_cursor_t *cursor = (map_cursor_t*) pCursor;
  ++(cursor->row);
  return SQLITE_OK;
}

int map_xEof(sqlite3_vtab_cursor* pCursor){
 map_table_t *table = (map_table_t*) pCursor->pVtab;
 map_cursor_t *cursor = (map_cursor_t*) pCursor;
 return cursor->row >= vec_length(table->content);
}

int map_sqlite3_pid(sqlite3_vtab_cursor* pCursor, sqlite3_context *ctx) {
  map_cursor_t *cursor = (map_cursor_t*) pCursor;
  map_table_t *table = (map_table_t*) pCursor->pVtab;
  map_t* map = (map_t*) vec_get(table->content,cursor->row);
  sqlite3_result_int(ctx, map->pid);
  return SQLITE_OK;
}

int map_sqlite3_map(sqlite3_vtab_cursor* pCursor, sqlite3_context *ctx) {
  map_cursor_t *cursor = (map_cursor_t*) pCursor;
  map_table_t *table = (map_table_t*) pCursor->pVtab;
  map_t* map = (map_t*) vec_get(table->content,cursor->row);
  sqlite3_result_text(ctx, map->map, map->map_len, SQLITE_STATIC);
  return SQLITE_OK;
}

int map_xColumn(sqlite3_vtab_cursor* pCursor, sqlite3_context* ctx, int col){
  assert(col <= 1);
  map_sqlite3_setters[col](pCursor, ctx);
  return SQLITE_OK;
}

int map_xRowid(sqlite3_vtab_cursor* pCursor, sqlite_int64 *pRowid){
  map_cursor_t *cursor = (map_cursor_t*) pCursor;
  *pRowid = cursor->row;
  return SQLITE_OK;
}
