all:
	gcc -I. -Wall main.c vector.c envvar.c sqlite_env.c -lsqlite3
