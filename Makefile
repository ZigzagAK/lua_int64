.PHONY: all

LUA_INCLUDE_DIR ?= /usr/local/include
LDFLAGS ?= -llua

all:
	gcc -O3 -g -Wall -fPIC --shared -o int64.so int64.c lua_mod64.c $(CFLAGS) -I$(LUA_INCLUDE_DIR) -fvisibility=hidden
	gcc -O3 -g -Wall -fPIC --shared -o liblua_int64.so lua_int64.c int64.c $(CFLAGS) -I$(LUA_INCLUDE_DIR) -fvisibility=hidden $(LDFLAGS)
