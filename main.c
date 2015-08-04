#include <stdlib.h>
#include <stdio.h>
#include <sqlite3.h>
#include <modules/env/sqlite3_env.h>
#include <modules/cmdline/sqlite3_cmdline.h>
#include <modules/exe/sqlite3_exe.h>

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
  int i = 0;
  while(i < argc){
    printf("%s = %s", azColName[i], argv[i] ? argv[i] : "NULL");
    while(++i < argc){
      printf(", %s = %s", azColName[i], argv[i] ? argv[i] : "NULL");
    }
  }
  printf("\n");
  return 0;
}

int main(int argc, char **argv){
  sqlite3 *db;
  char *zErrMsg = 0;
  int rc;


  rc = sqlite3_open(":memory:", &db);
  if( rc ){
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    return(1);
  }

  rc = sqlite3_create_module(db,"env",&env_module,0);
  if( rc!=SQLITE_OK ){
    fprintf(stderr,"error registering 'env' module with rc %i\n",rc);
    return 1;
  }
    
  rc = sqlite3_exec(db, "CREATE VIRTUAL TABLE env USING env", 0, 0, 0);
  if( rc!=SQLITE_OK ){
    fprintf(stderr,"error creating virtual table 'env' with rc %i\n",rc);
    return 1;
  }
  
  rc = sqlite3_create_module(db,"cmdline",&cmdline_module,0);
  if( rc!=SQLITE_OK ){
    fprintf(stderr,"error registering 'cmdline' module with rc %i\n",rc);
    return 1;
  }
    
  rc = sqlite3_exec(db, "CREATE VIRTUAL TABLE cmdline USING cmdline", 0, 0, 0);
  if( rc!=SQLITE_OK ){
    fprintf(stderr,"error creating virtual table 'cmdline' with rc %i\n",rc);
    return 1;
  }
  
  rc = sqlite3_create_module(db,"exe",&exe_module,0);
  if( rc!=SQLITE_OK ){
    fprintf(stderr,"error registering 'exe' module with rc %i\n",rc);
    return 1;
  }
    
  rc = sqlite3_exec(db, "CREATE VIRTUAL TABLE exe USING exe", 0, 0, 0);
  if( rc!=SQLITE_OK ){
    fprintf(stderr,"error creating virtual table 'exe' with rc %i\n",rc);
    return 1;
  }

  char default_query[] = "SELECT * from env";
  char* query = default_query;
  if(argc > 1) {
    query = argv[1];
  }
  printf("query: %s\n",query);
  rc = sqlite3_exec(db, query, callback, 0, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }
  sqlite3_close(db);
  return 0;
}
