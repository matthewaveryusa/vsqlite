#ifndef SQLITE3_ENV_H
#define SQLITE3_ENV_H

#include <sqlite3.h>

typedef struct env_table {
  sqlite3_vtab base;
  void *content;
} env_table_t;

typedef struct env_cursor {
  sqlite3_vtab_cursor base;
  int row;
} env_cursor_t;

#define ENV_PID_COLUMN 0
#define ENV_KEY_COLUMN 1
#define ENV_VALUE_COLUMN 2

int env_sqlite3_pid(sqlite3_vtab_cursor* pCursor, sqlite3_context *ctx) ;
int env_sqlite3_name(sqlite3_vtab_cursor* pCursor, sqlite3_context *ctx) ;
int env_sqlite3_value(sqlite3_vtab_cursor* pCursor, sqlite3_context *ctx) ;

int env_xCreate(sqlite3* db, void *pAux, int argc, const char *const*argv, sqlite3_vtab **ppVTab, char **pzErr);
int env_xBestIndex(sqlite3_vtab *pVTab, sqlite3_index_info* index_info);
int env_xDestroy(sqlite3_vtab *pVTab);
int env_xOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor);
int env_xFilter(sqlite3_vtab_cursor* pCursor, int idxNum, const char *idxStr, int argc, sqlite3_value **argv);
int env_xNext(sqlite3_vtab_cursor* pCursor);
int env_xEof(sqlite3_vtab_cursor* pCursor);
int env_xColumn(sqlite3_vtab_cursor* pCursor, sqlite3_context* ctx, int col);
int env_xRowid(sqlite3_vtab_cursor* pCursor, sqlite_int64 *pRowid);
int env_xClose(sqlite3_vtab_cursor* cursor);

extern int(*env_sqlite3_setters[])(sqlite3_vtab_cursor*, sqlite3_context*);
extern sqlite3_module env_module;

#endif
