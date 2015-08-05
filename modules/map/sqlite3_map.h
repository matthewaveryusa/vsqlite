#ifndef SQLITE3_MAP_H
#define SQLITE3_MAP_H

#include <sqlite3.h>

typedef struct map_table {
  sqlite3_vtab base;
  void *content;
} map_table_t;

typedef struct map_cursor {
  sqlite3_vtab_cursor base;
  int row;
} map_cursor_t;

#define MAP_PID_COLUMN 0
#define MAP_MAP_COLUMN 1

int map_sqlite3_pid(sqlite3_vtab_cursor* pCursor, sqlite3_context *ctx) ;
int map_sqlite3_map(sqlite3_vtab_cursor* pCursor, sqlite3_context *ctx) ;

int map_xCreate(sqlite3* db, void *pAux, int argc, const char *const*argv, sqlite3_vtab **ppVTab, char **pzErr);
int map_xBestIndex(sqlite3_vtab *pVTab, sqlite3_index_info* index_info);
int map_xDestroy(sqlite3_vtab *pVTab);
int map_xOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor);
int map_xFilter(sqlite3_vtab_cursor* pCursor, int idxNum, const char *idxStr, int argc, sqlite3_value **argv);
int map_xNext(sqlite3_vtab_cursor* pCursor);
int map_xEof(sqlite3_vtab_cursor* pCursor);
int map_xColumn(sqlite3_vtab_cursor* pCursor, sqlite3_context* ctx, int col);
int map_xRowid(sqlite3_vtab_cursor* pCursor, sqlite_int64 *pRowid);
int map_xClose(sqlite3_vtab_cursor* cursor);

extern int(*map_sqlite3_setters[])(sqlite3_vtab_cursor*, sqlite3_context*);
extern sqlite3_module map_module;

#endif
