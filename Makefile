.PHONY: all

LUA_INCLUDE_DIR ?= /usr/local/include
LDFLAGS ?= -llua

platform := $(shell uname)

ifeq ($(platform),Darwin)
  shared := dylib
else
  shared := so
endif

all:
	gcc -O3 -g -Wall -fPIC --shared -o int64.so int64.c lua_mod64.c $(CFLAGS) -I$(LUA_INCLUDE_DIR) -fvisibility=hidden $(LDFLAGS)
	gcc -O3 -g -Wall -fPIC --shared -o liblua_int64.$(shared) lua_int64.c int64.c $(CFLAGS) -I$(LUA_INCLUDE_DIR) -fvisibility=hidden $(LDFLAGS)
