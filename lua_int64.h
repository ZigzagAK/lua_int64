#ifndef _LUA_INT64_H_
#define _LUA_INT64_H_

#include <lua.h>
#include <inttypes.h>

extern const char *INT64;
extern const char *UINT64;

int lua_pushinteger64(lua_State *, int64_t);
int lua_pushunsigned64(lua_State *, uint64_t);
int64_t lua_getinteger64(void *);
uint64_t lua_getunsigned64(void *);

#endif
