#include "Common.h"
#include "Renderer.h"
#include "Menu.h"

#define LUA_LIB
#include "Scripting/Lua/lprefix.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "Scripting/Lua/lua.h"
#include "Scripting/Lua/ldebug.h"
#include "Scripting/Lua/lauxlib.h"
#include "Scripting/Lua/lualib.h"


template<size_t N = 80 * 40>
static FakeString<N> randFill(uint8_t min, uint8_t max, uint32_t seed = 123456)
{
    if(min > max) std::swap(min, max);
    uint32_t next = seed;
    FakeString<N> ret;
    uint8_t range = max - min;
    for(size_t i = 0; i < N; i++)
    {
        ret[i] = min + (const_rand(next) % range);
    }
    return ret;
}

FakeString<80*40> randPrintStr;

static int luaS_RandFillReroll(lua_State *L)
{
    checkargs("screen.RandFillReroll", 0);
    
    randPrintStr = randFill(1, 254, Util::MsTime());
    
    return 0;
}

static int luaS_RandFillPrint(lua_State *L)
{
    checkargs("screen.RandFillPrint", 0);
    
    Renderer::SetText(randPrintStr);
    
    return 0;
}

static int luaS_HighRes(lua_State *L)
{
    checkargs("screen.HighRes", 0);
    
    Renderer::HighRes();
    
    return 0;
}

static int luaS_LowRes(lua_State *L)
{
    checkargs("screen.LowRes", 0);
    
    Renderer::LowRes();
    
    return 0;
}

static int luaS_DrawClear(lua_State *L)
{
    checkargs("screen.DrawClear", 2);
    
    int c = lua_tointeger(L, 1);
    int prop = lua_tointeger(L, 2);
    
    Renderer::DrawClear(c, prop);
    
    lua_pop(L, nargs);
    return 0;
}

static int luaS_DrawBorderSingle(lua_State *L)
{
    checkargs("screen.DrawBorderSingle", 5);
    
    int prop = lua_tointeger(L, 1);
    int x = lua_tointeger(L, 2);
    int y = lua_tointeger(L, 3);
    int w = lua_tointeger(L, 4);
    int h = lua_tointeger(L, 5);
    
    Menu::DrawBorderSingle(prop, x, y, w, h);
    
    lua_pop(L, nargs);
    return 0;
}

static int luaS_DrawBorderDouble(lua_State *L)
{
    checkargs("screen.DrawBorderDouble", 5);
    
    int prop = lua_tointeger(L, 1);
    int x = lua_tointeger(L, 2);
    int y = lua_tointeger(L, 3);
    int w = lua_tointeger(L, 4);
    int h = lua_tointeger(L, 5);
    
    Menu::DrawBorderDouble(prop, x, y, w, h);
    
    lua_pop(L, nargs);
    return 0;
}

static int luaS_DrawFillLineProp(lua_State *L)
{
    checkargs("screen.DrawFillLineProp", 4);
    
    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    int prop = lua_tointeger(L, 3);
    int width = lua_tointeger(L, 4);
    
    Renderer::DrawFillLineProp(x, y, prop, width);
    
    lua_pop(L, nargs);
    return 0;
}

//void DrawLineText(uint32_t x, uint32_t y, std::string_view newText, uint32_t width = 0);

static int luaS_DrawLineText(lua_State *L)
{
    checkargs2("screen.DrawLineText", 3, 4);
    
    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    std::string_view newText = lua_tostring_view(L, 3);
    int width = nargs < 4 ? 0 : lua_tointeger(L, 4);
    
    Renderer::DrawLineText(x, y, newText, width);
    
    lua_pop(L, nargs);
    return 0;
}

static int luaS_DrawLineTextFillProp(lua_State *L)
{
    checkargs2("screen.DrawLineTextFillProp", 4, 5);
    
    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    std::string_view newText = lua_tostring_view(L, 3);
    int newProp = lua_tointeger(L, 4);
    int width = nargs < 5 ? 0 : lua_tointeger(L, 5);
    
    Renderer::DrawLineTextFillProp(x, y, newText, newProp, width);
    
    lua_pop(L, nargs);
    return 0;
}

static int luaS_DrawLineTextCentered(lua_State *L)
{
    checkargs("screen.DrawLineTextCentered", 2);
    
    int y = lua_tointeger(L, 1);
    std::string_view str = lua_tostring_view(L, 2);
    
    Renderer::DrawLineTextCentered(y, str);
    
    lua_pop(L, nargs);
    return 0;
}

static int luaS_DrawTextBox(lua_State *L)
{
    checkargs("screen.DrawTextBox", 4);
    
    int y = lua_tointeger(L, 1);
    std::string_view str = lua_tostring_view(L, 2);
    std::string_view str_end = lua_tostring_view(L, 3);
    bool end_inside = lua_toboolean(L, 4);
    
    int result = Menu::DrawTextBox(y, str, str_end, end_inside);
    
    lua_pop(L, nargs);
    
    lua_pushnumber(L, result);
    return 1;
}

static const luaL_Reg screen_funcs[]
{
    {"DrawClear", luaS_DrawClear},
    {"RandFillReroll", luaS_RandFillReroll},
    {"RandFillPrint", luaS_RandFillPrint},
    {"HighRes", luaS_HighRes},
    {"LowRes", luaS_LowRes},
    {"DrawBorderSingle", luaS_DrawBorderSingle},
    {"DrawBorderDouble", luaS_DrawBorderDouble},
    {"DrawFillLineProp", luaS_DrawFillLineProp},
    {"DrawLineText", luaS_DrawLineText},
    {"DrawLineTextFillProp", luaS_DrawLineTextFillProp},
    {"DrawLineTextCentered", luaS_DrawLineTextCentered},
    {"DrawTextBox", luaS_DrawTextBox},
    {"CHAR_INVERT1", NULL},
    {"CHAR_INVERT2", NULL},
    {"CHAR_BLINK_INVERT", NULL},
    {"CHAR_UNDERSCORE", NULL},
    {"CHAR_BLINK1", NULL},
    {"CHAR_BLINK2", NULL},
    {"CHAR_BLINK3", NULL},
    {NULL, NULL}
};

LUAMOD_API int luaopen_screen(lua_State *L)
{
    luaL_newlib(L, screen_funcs);
    
    lua_pushinteger(L, CHAR_INVERT1);
    lua_setfield(L, -2, "CHAR_INVERT1");
    
    lua_pushinteger(L, CHAR_INVERT2);
    lua_setfield(L, -2, "CHAR_INVERT2");
    
    lua_pushinteger(L, CHAR_BLINK_INVERT);
    lua_setfield(L, -2, "CHAR_BLINK_INVERT");
    
    lua_pushinteger(L, CHAR_UNDERSCORE);
    lua_setfield(L, -2, "CHAR_UNDERSCORE");
    
    lua_pushinteger(L, CHAR_BLINK1);
    lua_setfield(L, -2, "CHAR_BLINK1");
    
    lua_pushinteger(L, CHAR_BLINK2);
    lua_setfield(L, -2, "CHAR_BLINK2");
    
    lua_pushinteger(L, CHAR_BLINK3);
    lua_setfield(L, -2, "CHAR_BLINK3");
    return 1;
}
