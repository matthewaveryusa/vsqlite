all:
	gcc -g -O0 -I. -Itools -Imodules/env -Wall main.c modules/env/env.c modules/env/sqlite3_env.c modules/cmdline/cmdline.c modules/cmdline/sqlite3_cmdline.c modules/exe/exe.c modules/exe/sqlite3_exe.c tools/vector.c -lsqlite3
