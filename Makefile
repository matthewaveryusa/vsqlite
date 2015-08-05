all:
	gcc -g -O0 -I. -Itools -Imodules/env -Wall -Wextra main.c modules/env/env.c modules/env/sqlite3_env.c modules/cmdline/cmdline.c modules/cmdline/sqlite3_cmdline.c modules/exe/exe.c modules/exe/sqlite3_exe.c tools/vector.c -lsqlite3 -Wno-unused-variable -Wno-unused-parameter -Wno-sign-compare -Wno-missing-field-initializers
