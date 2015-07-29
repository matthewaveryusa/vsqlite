all:
	gcc -I. -Itools -Imodules/env -Wall main.c modules/env/env.c modules/env/sqlite3_env.c tools/vector.c -lsqlite3
