#include <lua.h>
#include <lauxlib.h>

extern const char *INT64;
extern const char *UINT64;

extern int open_int64(lua_State *L);

__attribute__ ((visibility ("default")))
int luaopen_int64(lua_State *L)
{
	return open_int64(L);
}
