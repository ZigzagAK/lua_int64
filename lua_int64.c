#include "lua_int64.h"

extern int push_i64(lua_State *, int64_t);
extern int push_u64(lua_State *, uint64_t);
extern int64_t get_i64(void *);
extern uint64_t get_u64(void *);

__attribute__ ((visibility ("default")))
int lua_pushinteger64(lua_State *L, int64_t v)
{
    return push_i64(L, v);
}

__attribute__ ((visibility ("default")))
int lua_pushunsigned64(lua_State *L, uint64_t v)
{
    return push_u64(L, v);
}

__attribute__ ((visibility ("default")))
int64_t lua_getinteger64(void *ud)
{
    return get_i64(ud);
}

__attribute__ ((visibility ("default")))
uint64_t lua_getunsigned64(void *ud)
{
    return get_u64(ud);
}
