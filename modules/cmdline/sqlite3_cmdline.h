#ifndef SQLITE3_CMDLINE_H
#define SQLITE3_CMDLINE_H

#include <sqlite3.h>

typedef struct cmdline_table {
  sqlite3_vtab base;
  void *content;
} cmdline_table_t;

typedef struct cmdline_cursor {
  sqlite3_vtab_cursor base;
  int row;
} cmdline_cursor_t;

int cmdline_sqlite3_pid(sqlite3_vtab_cursor* pCursor, sqlite3_context *ctx) ;
int cmdline_sqlite3_cmdline(sqlite3_vtab_cursor* pCursor, sqlite3_context *ctx) ;

int cmdline_xCreate(sqlite3* db, void *pAux, int argc, const char *const*argv, sqlite3_vtab **ppVTab, char **pzErr);
int cmdline_xBestIndex(sqlite3_vtab *pVTab, sqlite3_index_info* index_info);
int cmdline_xDestroy(sqlite3_vtab *pVTab);
int cmdline_xOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor);
int cmdline_xFilter(sqlite3_vtab_cursor* pCursor, int idxNum, const char *idxStr, int argc, sqlite3_value **argv);
int cmdline_xNext(sqlite3_vtab_cursor* pCursor);
int cmdline_xEof(sqlite3_vtab_cursor* pCursor);
int cmdline_xColumn(sqlite3_vtab_cursor* pCursor, sqlite3_context* ctx, int col);
int cmdline_xRowid(sqlite3_vtab_cursor* pCursor, sqlite_int64 *pRowid);
int cmdline_xClose(sqlite3_vtab_cursor* cursor);

extern int(*cmdline_sqlite3_setters[])(sqlite3_vtab_cursor*, sqlite3_context*);
extern sqlite3_module cmdline_module;

#endif
