#ifndef SQLITE3_EXE_H
#define SQLITE3_EXE_H

#include <sqlite3.h>

typedef struct exe_table {
  sqlite3_vtab base;
  void *content;
} exe_table_t;

typedef struct exe_cursor {
  sqlite3_vtab_cursor base;
  int row;
} exe_cursor_t;

#define EXE_PID_COLUMN 0
#define EXE_EXE_COLUMN 1

int exe_sqlite3_pid(sqlite3_vtab_cursor* pCursor, sqlite3_context *ctx) ;
int exe_sqlite3_exe(sqlite3_vtab_cursor* pCursor, sqlite3_context *ctx) ;

int exe_xCreate(sqlite3* db, void *pAux, int argc, const char *const*argv, sqlite3_vtab **ppVTab, char **pzErr);
int exe_xBestIndex(sqlite3_vtab *pVTab, sqlite3_index_info* index_info);
int exe_xDestroy(sqlite3_vtab *pVTab);
int exe_xOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor);
int exe_xFilter(sqlite3_vtab_cursor* pCursor, int idxNum, const char *idxStr, int argc, sqlite3_value **argv);
int exe_xNext(sqlite3_vtab_cursor* pCursor);
int exe_xEof(sqlite3_vtab_cursor* pCursor);
int exe_xColumn(sqlite3_vtab_cursor* pCursor, sqlite3_context* ctx, int col);
int exe_xRowid(sqlite3_vtab_cursor* pCursor, sqlite_int64 *pRowid);
int exe_xClose(sqlite3_vtab_cursor* cursor);

extern int(*exe_sqlite3_setters[])(sqlite3_vtab_cursor*, sqlite3_context*);
extern sqlite3_module exe_module;

#endif
