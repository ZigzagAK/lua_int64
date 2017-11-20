#ifndef _INT64_H_
#define _INT64_H_

#include <inttypes.h>


extern const char *INT64;
extern const char *UINT64;


int
push_i64(lua_State *L, int64_t v);


int
push_u64(lua_State *L, uint64_t v);


int64_t
get_i64(void *ud);


uint64_t
get_u64(void *ud);


#endif
