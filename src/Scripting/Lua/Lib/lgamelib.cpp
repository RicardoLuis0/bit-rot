#include "Common.h"
#include "Config.h"
#include "Input.h"
#include "Game.h"
#include "SaveData.h"

#define LUA_LIB
#include "Scripting/Lua/lprefix.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>

#include "Scripting/Lua/lua.h"
#include "Scripting/Lua/ldebug.h"
#include "Scripting/Lua/lauxlib.h"
#include "Scripting/Lua/lualib.h"


static int luaG_MsTime(lua_State *L)
{
    checkargs("game.MsTime", 0);
    
    lua_pushnumber(L, Util::MsTime());
    return 1;
}

static int luaG_IsLoadingSave(lua_State *L)
{
    checkargs("game.IsLoadingSave", 0);
    
    lua_pushboolean(L, Game::GameIsSave);
    return 1;
}

static int luaG_HasSave(lua_State *L)
{
    checkargs("game.HasSave", 0);
    
    lua_pushboolean(L, SaveData::HasSave());
    return 1;
}

static int luaG_ToGame(lua_State *L)
{
    checkargs("game.ToGame", 0);
    Game::ToGame();
    return 0;
}

extern std::map<std::string, std::string> texts;

static int luaG_GetText(lua_State *L)
{
    checkargs("game.GetText", 1);
    
    std::string index = lua_tostring(L, 1);
    
    if(texts.find(index) != texts.end())
    {
        lua_pop(L, nargs);
        std::string s = texts[index];
        lua_pushlstring(L, s.c_str(), s.length());
        return 1;
    }
    else
    {
        lua_pop(L, nargs);
        luaG_runerror(L, ("Trying to get inexistent text '" + index + "' in game.GetText").c_str());
        return 0;
    }
}

static int luaG_Log(lua_State *L)
{
    checkargs("game.Log", 1);
    
    std::string msg = lua_tostring(L, 1);
    
    Log::LuaLogFull(LogPriority::INFO, "", "", "", 0, msg);
    
    return 0;
}

static int luaC_GetStringOr(lua_State *L)
{
    checkargs("config.GetStringOr", 2);
    
    std::string key = lua_tostring(L, 1);
    std::string alternative = lua_tostring(L, 2);
    
    lua_pop(L, nargs);
    
    std::string_view s = Config::getScriptStringOr(key, alternative);
    lua_pushlstring(L, s.data(), s.length());
    return 1;
}

static int luaA_PlaySample(lua_State *L)
{
    checkargs("audio.PlaySample", 1);
    
    std::string sample(Util::StrToLower(lua_tostring_view(L, 1)));
    
    lua_pop(L, nargs);
    
    if(sample == "beep")
    {
        Audio::Beep();
    }
    else if(sample == "error")
    {
        Audio::Error();
    }
    else
    {
        luaG_runerror(L, ("Trying to play inexistent sample '" + sample + "' in audio.PlaySample").c_str());
        return 0;
    }
    
    return 0;
}

static int luaA_StartLoop(lua_State *L)
{
    checkargs("audio.StartLoop", 1);
    
    std::string loop(Util::StrToLower(lua_tostring_view(L, 1)));
    
    lua_pop(L, nargs);
    
    if(loop == "fan")
    {
        Audio::StartFan();
    }
    else
    {
        luaG_runerror(L, ("Trying to start inexistent loop '" + loop + "' in audio.StartLoop").c_str());
        return 0;
    }
    
    return 0;
}


static int luaA_PlayMusic(lua_State *L)
{
    checkargs("audio.PlayMusic", 1);
    
    std::string music(Util::StrToLower(lua_tostring_view(L, 1)));
    
    lua_pop(L, nargs);
    
    try
    {
        Audio::PlayMusic(music);
    }
    catch(std::out_of_range &e)
    {
        luaG_runerror(L, ("Trying to play inexistent music '" + music + "' in audio.PlayMusic").c_str());
        return 0;
    }
    
    return 0;
}

static int luaA_FadeMusic(lua_State *L)
{
    checkargs("audio.FadeMusic", 1);
    
    Audio::FadeMusic(lua_tonumber(L, 1));
    
    lua_pop(L, nargs);
    return 0;
}

static const luaL_Reg game_funcs[]
{
    {"MsTime", luaG_MsTime},
    {"GetText", luaG_GetText},
    {"IsLoadingSave", luaG_IsLoadingSave},
    {"HasSave", luaG_HasSave},
    {"ToGame", luaG_ToGame},
    {"Log", luaG_Log},
    {NULL, NULL}
};

static const luaL_Reg config_funcs[]
{
    {"GetStringOr", luaC_GetStringOr},
    {NULL, NULL}
};

static const luaL_Reg audio_funcs[]
{
    {"PlaySample", luaA_PlaySample},
    {"StartLoop", luaA_StartLoop},
    {"PlayMusic", luaA_PlayMusic},
    {"FadeMusic", luaA_FadeMusic},
    {NULL, NULL}
};

LUAMOD_API int luaopen_game(lua_State *L)
{
    luaL_newlib(L, game_funcs);
    return 1;
}

LUAMOD_API int luaopen_config(lua_State *L)
{
    luaL_newlib(L, config_funcs);
    return 1;
}

LUAMOD_API int luaopen_audio(lua_State *L)
{
    luaL_newlib(L, audio_funcs);
    return 1;
}
