#include <lua.h>
#include <lauxlib.h>
#include <math.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <assert.h>

#if !defined LUA_VERSION_NUM || LUA_VERSION_NUM < 502

static void
luaL_setfuncs(lua_State *l, const luaL_Reg *reg, int nup)
{
    int i;

    luaL_checkstack(l, nup, "too many upvalues");
    for (; reg->name != NULL; reg++) {
        for (i = 0; i < nup; i++) {
            lua_pushvalue(l, -nup);
        }
        lua_pushcclosure(l, reg->func, nup);
        lua_setfield(l, -(nup + 2), reg->name);
    }
    lua_pop(l, nup);
}

static void *
luaL_testudata(lua_State *L, int i, const char *tname)
{
    void *p = lua_touserdata(L, i);
    luaL_checkstack(L, 2, "not enough stack slots");
    if (p == NULL || !lua_getmetatable(L, i)) {
        return NULL;
    } else {
        int res = 0;
        luaL_getmetatable(L, tname);
        res = lua_rawequal(L, -1, -2);
        lua_pop(L, 2);
        if (!res) {
           p = NULL;
        }
    }
    return p;
}

#define luaL_newlibtable(L,l) (lua_createtable(L,0,sizeof(l)))
#define luaL_newlib(L,l) (luaL_newlibtable(L,l), luaL_setfuncs(L,l,0))

#endif


const char *INT64  = "INT64";
const char *UINT64 = "UINT64";


#include "int64.h"


struct ud64_s {
    const char *type;
    union {
        int64_t  INT64;
        uint64_t UINT64;
    } value;
};
typedef struct ud64_s ud64_t;


#define UD64_GET(ud) (                       \
    (ud)->type == INT64 ? (ud)->value.INT64  \
                        : (ud)->value.UINT64 \
)


static void
ud64_copy_val(ud64_t *dst, ud64_t *src)
{
    memcpy((void *) &dst->value, (void *) &src->value, sizeof(dst->value));
}


static int
ud64_lua_parse(lua_State *L, int index, ud64_t *ud)
{
    size_t len = 0;
    char *endptr;
    const char *str = (const char *) lua_tolstring(L, index, &len);

    assert(ud->type == INT64 || ud->type == UINT64);

    if (ud->type == INT64) {
        ud->value.INT64  = (int64_t) strtoll(str, &endptr, 10);
        if ((errno == ERANGE && (ud->value.INT64 == LLONG_MAX || ud->value.INT64 == LLONG_MIN))
                   || (errno != 0 && ud->value.INT64 == 0)) {
            return luaL_error(L, "The string (length = %d) is not an int64 string", len);
        }
    } else {
        ud->value.UINT64 = (uint64_t) strtoull(str, &endptr, 10);
        if ((errno == ERANGE && (ud->value.UINT64 == ULLONG_MAX || ud->value.UINT64 == 0))
                   || (errno != 0 && ud->value.UINT64 == 0)) {
            return luaL_error(L, "The string (length = %d) is not an int64 string", len);
        }
    }

    return 1;
}


static const char *
ud64_lua_set(lua_State *L, int index, ud64_t *ud)
{
    ud64_t *src = (ud64_t *) luaL_testudata(L, index, INT64);
    if (src) {
        ud64_copy_val(ud, src);
        return INT64;
    }

    src = (ud64_t *) luaL_testudata(L, index, UINT64);
    if (src) {
        ud64_copy_val(ud, src);
        return UINT64;
    }

    return 0;
}


static int
ud64_lua_init(lua_State *L, ud64_t *ud)
{
    int type = lua_type(L, 1);

    switch (type) {
        case LUA_TNUMBER:
            ud->value.INT64 = lua_tointeger(L, 1);
            break;

        case LUA_TSTRING:
            ud64_lua_parse(L, 1, ud);
            break;

        case LUA_TUSERDATA:
          if (ud64_lua_set(L, 1, ud)) {
              break;
          }

        default:
            return luaL_error(L, "argument error type [%s]", lua_typename(L, type));
    }

    return 1;
}


static ud64_t*
ud64_new(lua_State *L, const char *type)
{
    ud64_t *ud = (ud64_t *) lua_newuserdata(L, sizeof(ud64_t));
    if (ud == NULL) {
        return NULL;
    }

    ud->type = type;
    bzero((void *) &ud->value, sizeof(ud->value));

    luaL_getmetatable(L, type);
    lua_setmetatable(L, -2);

    return ud;
}


static int
ud64_lua_new(lua_State *L, const char *type)
{
    int     top;
    ud64_t *ud;

    top = lua_gettop(L);

    ud = ud64_new(L, type);
    if (ud == NULL) {
        return luaL_error(L, "no memory");
    }

    return top == 0 ? 1 : ud64_lua_init(L, ud);
}


static int
ud64_lua_new_i64(lua_State *L)
{
    return ud64_lua_new(L, INT64);
}


static int
ud64_lua_new_u64(lua_State *L)
{
    return ud64_lua_new(L, UINT64);
}


static int
ud64_lua_value(lua_State *L, int index, ud64_t *ud)
{
    int type = lua_type(L, index);

    switch (type) {
        case LUA_TUSERDATA:
            ud->type = ud64_lua_set(L, index, ud);
            if (ud->type) {
                return UD64_GET(ud);
            }
            break;

        case LUA_TNUMBER:
            ud->type = INT64;
            ud->value.INT64 = lua_tointeger(L, index);
            return ud->value.INT64;
    }

    return luaL_error(L, "argument error type [%s]", lua_typename(L, type));
}


#define DECLARE_METHOD(M, TYPE, OP)                      \
static int                                               \
op_ ## M ##_## TYPE (lua_State *L)                       \
{                                                        \
    ud64_t  lhs;                                         \
    ud64_t  rhs;                                         \
    ud64_t *out;                                         \
                                                         \
    if (lua_gettop(L) != 2) {                            \
        return luaL_error(L, "two arguments required");  \
    }                                                    \
                                                         \
    ud64_lua_value(L, 1, &lhs);                          \
    ud64_lua_value(L, 2, &rhs);                          \
                                                         \
    if (NULL == (out = ud64_new(L, TYPE))) {             \
        return luaL_error(L, "no memory");               \
    }                                                    \
                                                         \
    out->value.TYPE = UD64_GET(&lhs) OP UD64_GET(&rhs);  \
                                                         \
    return 1;                                            \
}


DECLARE_METHOD(add, INT64, +)
DECLARE_METHOD(sub, INT64, -)
DECLARE_METHOD(mul, INT64, *)
DECLARE_METHOD(div, INT64, /)
DECLARE_METHOD(mod, INT64, %)


DECLARE_METHOD(add, UINT64, +)
DECLARE_METHOD(sub, UINT64, -)
DECLARE_METHOD(mul, UINT64, *)
DECLARE_METHOD(div, UINT64, /)
DECLARE_METHOD(mod, UINT64, %)


#define DECLARE_POW(TYPE)                                  \
static int                                                 \
pow_ ## TYPE (lua_State *L)                                \
{                                                          \
    ud64_t  lhs;                                           \
    ud64_t  rhs;                                           \
    ud64_t *out;                                           \
                                                           \
    if (lua_gettop(L) != 2) {                              \
        return luaL_error(L, "two arguments required");    \
    }                                                      \
                                                           \
    ud64_lua_value(L, 1, &lhs);                            \
    ud64_lua_value(L, 2, &rhs);                            \
                                                           \
    if (NULL == (out = ud64_new(L, TYPE))) {               \
        return luaL_error(L, "no memory");                 \
    }                                                      \
                                                           \
    out->value.TYPE = pow(UD64_GET(&lhs), UD64_GET(&rhs)); \
                                                           \
    return 1;                                              \
}


DECLARE_POW(INT64)
DECLARE_POW(UINT64)


static int
op_unm(lua_State *L)
{
    ud64_t  *lhs;
    ud64_t  *out;

    if (lua_gettop(L) == 0) {
        return luaL_error(L, "one argument required");
    }

    lhs = luaL_checkudata(L, 1, INT64);

    out = ud64_new(L, INT64);
    if (out == NULL) {
        return luaL_error(L, "no memory");
    }

    out->value.INT64 = -lhs->value.INT64;

    return 1;
}


#define DECLARE_CMP(M, TYPE, OP)                          \
static int                                                \
op_ ## M ##_## TYPE (lua_State *L)                        \
{                                                         \
    ud64_t lhs;                                           \
    ud64_t rhs;                                           \
                                                          \
    if (lua_gettop(L) != 2) {                             \
        return luaL_error(L, "two arguments required");   \
    }                                                     \
                                                          \
    ud64_lua_value(L, 1, &lhs);                           \
    ud64_lua_value(L, 2, &rhs);                           \
                                                          \
    lua_pushboolean(L, UD64_GET(&lhs) OP UD64_GET(&rhs)); \
                                                          \
    return 1;                                             \
}


DECLARE_CMP(lt, INT64, <)
DECLARE_CMP(le, INT64, <=)

DECLARE_CMP(lt, UINT64, <)
DECLARE_CMP(le, UINT64, <=)


static int
op_eq(lua_State *L)
{
    ud64_t lhs;
    ud64_t rhs;

    if (lua_gettop(L) != 2) {
        return luaL_error(L, "two arguments required");
    }

    ud64_lua_value(L, 1, &lhs);
    ud64_lua_value(L, 2, &rhs);

    lua_pushboolean(L, UD64_GET(&lhs) == UD64_GET(&rhs));

    return 1;
}


static int
op_len(lua_State *L)
{
    lua_pushnumber(L, sizeof(ptrdiff_t));
    return 1;
}


static int
tonumber(lua_State *L)
{
    ud64_t lhs;

    if (lua_gettop(L) == 0) {
        return luaL_error(L, "one argument required");
    }

    ud64_lua_value(L, 1, &lhs);

    lua_pushnumber(L, (lua_Number) UD64_GET(&lhs));

    return 1;
}


static int
compare(lua_State *L)
{
    ud64_t lhs;
    ud64_t rhs;

    if (lua_gettop(L) != 2) {
        return luaL_error(L, "two arguments required");
    }

    ud64_lua_value(L, 1, &lhs);
    ud64_lua_value(L, 2, &rhs);

    lua_pushnumber(L, UD64_GET(&lhs) > UD64_GET(&rhs) ? 1
        : UD64_GET(&lhs) < UD64_GET(&rhs) ? -1 : 0);

    return 1;
}


#define DECLARE_TOSTRING(TYPE, FMT)                       \
static int                                                \
tostring_ ## TYPE (lua_State *L)                          \
{                                                         \
    ud64_t *lhs;                                          \
    char    buffer[23];                                   \
    int     len;                                          \
                                                          \
    if (lua_gettop(L) != 1) {                             \
        return luaL_error(L, "one argument required");    \
    }                                                     \
                                                          \
    lhs = (ud64_t *)luaL_checkudata(L, 1, #TYPE);         \
                                                          \
    len = snprintf(buffer, 22, "%" FMT, lhs->value.TYPE); \
    if (len < 0) {                                        \
        return luaL_error(L, "invalid argument");         \
    }                                                     \
                                                          \
    lua_pushlstring(L, (const char *) buffer, len);       \
                                                          \
    return 1;                                             \
}


DECLARE_TOSTRING(INT64, PRId64)
DECLARE_TOSTRING(UINT64, PRIu64)


#define DECLARE_CONCAT(TYPE, FMT)                         \
static int                                                \
concat_ ## TYPE (lua_State *L)                            \
{                                                         \
    const char *str;                                      \
    ud64_t     *ud64;                                     \
    char       *buffer;                                   \
    size_t      len = 0, sz;                              \
    int         type1;                                    \
                                                          \
    if (lua_gettop(L) != 2) {                             \
        return luaL_error(L, "two arguments required");   \
    }                                                     \
                                                          \
    type1 = lua_type(L, 1);                               \
                                                          \
    if (type1 == LUA_TSTRING) {                           \
        str = (const char *) lua_tolstring(L, 1, &len);   \
        ud64 = (ud64_t *) luaL_checkudata(L, 2, #TYPE);   \
    } else {                                              \
        ud64 = (ud64_t *) luaL_checkudata(L, 1, #TYPE);   \
        str = (const char *) lua_tolstring(L, 2, &len);   \
    }                                                     \
                                                          \
    buffer = (char *) (len > 10217 ? malloc(len + 23)     \
                                   : alloca(len + 23));   \
    if (buffer == NULL) {                                 \
        return luaL_error(L, "no memory");                \
    }                                                     \
                                                          \
    if (type1 == LUA_TSTRING) {                           \
        sz = snprintf(buffer, len + 22, "%s%" FMT,        \
                      str, ud64->value.TYPE);             \
    } else {                                              \
        sz = snprintf(buffer, len + 22, "%" FMT "%s",     \
                      ud64->value.TYPE, str);             \
    }                                                     \
                                                          \
    if (sz < 0) {                                         \
        if (len > 10217) {                                \
            free(buffer);                                 \
        }                                                 \
        return luaL_error(L, "invalid argument");         \
    }                                                     \
                                                          \
    lua_pushlstring(L, (const char *) buffer, sz);        \
                                                          \
    if (len > 10217) {                                    \
        free(buffer);                                     \
    }                                                     \
                                                          \
    return 1;                                             \
}


DECLARE_CONCAT(INT64, PRId64)
DECLARE_CONCAT(UINT64, PRIu64)


static luaL_Reg
lib_int64[] = {
    { "__add", op_add_INT64 },
    { "__sub", op_sub_INT64 },
    { "__mul", op_mul_INT64 },
    { "__div", op_div_INT64 },
    { "__mod", op_mod_INT64 },
    { "__unm", op_unm       },
    { "__pow", pow_INT64    },
    { "__eq",  op_eq        },
    { "__lt",  op_lt_INT64  },
    { "__le",  op_le_INT64  },
    { "__len", op_len       },
    { "__tostring",
             tostring_INT64 },
    { "__concat",
               concat_INT64 },
    { "compare", compare    },
    { "tonumber", tonumber  },
    { NULL, NULL }
};


static luaL_Reg
lib_uint64[] = {
    { "__add", op_add_UINT64 },
    { "__sub", op_sub_UINT64 },
    { "__mul", op_mul_UINT64 },
    { "__div", op_div_UINT64 },
    { "__mod", op_mod_UINT64 },
    { "__unm", op_unm        },
    { "__pow", pow_UINT64    },
    { "__eq",  op_eq         },
    { "__lt",  op_lt_UINT64  },
    { "__le",  op_le_UINT64  },
    { "__len", op_len        },
    { "__tostring",
             tostring_UINT64 },
    { "__concat",
               concat_UINT64 },
    { "compare", compare     },
    { "tonumber", tonumber   },
    { NULL, NULL }
};


static const luaL_Reg
funcs[] = {
    { "signed",   ud64_lua_new_i64 },
    { "unsigned", ud64_lua_new_u64 },
    { NULL, NULL }
};


/******************** lua library ********************/


int
luaopen_int64(lua_State *L)
{
    luaL_newmetatable(L, INT64);
    luaL_setfuncs(L, lib_int64, 0);
    lua_setfield(L, -1, "__index");

    luaL_newmetatable(L, UINT64);
    luaL_setfuncs(L, lib_uint64, 0);
    lua_setfield(L, -1, "__index");

    luaL_newlib(L, funcs);

    return 1;
}


/******************** public API ********************/


int
push_i64(lua_State *L, int64_t v)
{
    ud64_t *ud = ud64_new(L, INT64);
    if (ud == NULL) {
        return luaL_error(L, "no memory");
    }

    ud->value.INT64 = v;

    return 1;
}


int
push_u64(lua_State *L, uint64_t v)
{
    ud64_t *out = ud64_new(L, UINT64);
    if (out == NULL) {
        return luaL_error(L, "no memory");
    }

    out->value.UINT64 = v;

    return 1;
}


int64_t
get_i64(void *ud)
{
    assert(ud);
    return ((ud64_t *) ud)->value.INT64;
}


uint64_t
get_u64(void *ud)
{
    assert(ud);
    return ((ud64_t *) ud)->value.UINT64;
}
