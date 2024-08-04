/*
** $Id: lualib.h $
** Lua standard libraries
** See Copyright Notice in lua.h
*/


#ifndef lualib_h
#define lualib_h

#include "Scripting/Lua/lua.h"


/* version suffix for environment variable names */
#define LUA_VERSUFFIX          "_" LUA_VERSION_MAJOR "_" LUA_VERSION_MINOR


LUAMOD_API int (luaopen_base) (lua_State *L);

#define LUA_COLIBNAME	"coroutine"
LUAMOD_API int (luaopen_coroutine) (lua_State *L);

#define LUA_TABLIBNAME	"table"
LUAMOD_API int (luaopen_table) (lua_State *L);

#define LUA_STRLIBNAME	"string"
LUAMOD_API int (luaopen_string) (lua_State *L);

#define LUA_UTF8LIBNAME	"utf8"
LUAMOD_API int (luaopen_utf8) (lua_State *L);

#define LUA_MATHLIBNAME	"math"
LUAMOD_API int (luaopen_math) (lua_State *L);

#define LUA_GAMELIBNAME	"game"
LUAMOD_API int (luaopen_game) (lua_State *L);

#define LUA_CONFIGLIBNAME	"config"
LUAMOD_API int (luaopen_config) (lua_State *L);

#define LUA_SCREENLIBNAME	"screen"
LUAMOD_API int (luaopen_screen) (lua_State *L);

#define LUA_AUDIOLIBNAME	"audio"
LUAMOD_API int (luaopen_audio) (lua_State *L);


/* open all previous libraries */
LUALIB_API void (luaL_openlibs) (lua_State *L);




#define checkargs2(NAME, NUM1, NUM2) \
    int nargs = lua_gettop(L); \
    if(nargs < NUM1) \
    { \
        lua_pop(L, nargs); \
        luaG_runerror(L, "Too few args passed to " NAME ", expected " #NUM1 " got %d", nargs); \
        return 0; \
    } \
    else if(nargs > NUM2) \
    { \
        lua_pop(L, nargs); \
        luaG_runerror(L, "Too many args passed to " NAME ", expected " #NUM2 " got %d", nargs); \
        return 0; \
    }


#define checkargs(NAME, NUM) checkargs2(NAME, NUM, NUM)

#endif
