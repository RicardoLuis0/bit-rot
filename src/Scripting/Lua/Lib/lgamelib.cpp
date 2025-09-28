#include "Common.h"
#include "Config.h"
#include "Input.h"
#include "Sound.h"
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
    
    lua_pushinteger(L, Util::MsTime());
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

static int luaG_ClearSave(lua_State *L)
{
    checkargs("game.ClearSave", 0);
    SaveData::Clear();
    return 0;
}

extern bool RunGame;

static int luaG_Close(lua_State *L)
{
    checkargs("game.Close", 0);
    RunGame = false;
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

static int luaG_GetInitText(lua_State *L)
{
    checkargs("game.GetInitText", 1);
    
    int index = lua_tointeger(L, 1);
    lua_pop(L, nargs);
    
    auto & txt = initText.at(index);
    
    lua_newtable(L);
    
    lua_pushinteger(L, txt.timer);
    lua_setfield(L, -2, "timer");
    
    lua_pushlstring(L, txt.text.c_str(), txt.text.length());
    lua_setfield(L, -2, "text");
    
    lua_pushboolean(L, txt.beep);
    lua_setfield(L, -2, "beep");
    
    lua_pushboolean(L, txt.recovery);
    lua_setfield(L, -2, "recovery");
    
    lua_pushboolean(L, txt.intro);
    lua_setfield(L, -2, "intro");
    
    return 1;
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

static int luaC_SetString(lua_State *L)
{
    checkargs("config.SetString", 2);
    
    std::string key = lua_tostring(L, 1);
    std::string value = lua_tostring(L, 2);
    
    lua_pop(L, nargs);
    
    Config::setScriptString(key, value);
    return 0;
}

static int luaA_PlaySample(lua_State *L)
{
    checkargs2("audio.PlaySample", 1, 3);
    
    std::string sample(Util::StrToLower(lua_tostring_view(L, 1)));
    bool overlap = false;
    bool loop = false;
    
    if(nargs > 1)
    {
        overlap = lua_toboolean(L, 2);
    }
    
    if(nargs > 2)
    {
        loop = lua_toboolean(L, 3);
    }
    
    lua_pop(L, nargs);
    
    Sound::PlaySFX(sample, overlap, loop);
    
    return 0;
}

static int luaA_IsSamplePlaying(lua_State *L)
{
    checkargs("audio.IsSamplePlaying", 1);
    
    std::string sample(Util::StrToLower(lua_tostring_view(L, 1)));
    
    lua_pop(L, nargs);
    
    lua_pushboolean(L, Sound::IsSamplePlaying(sample));
    return 1;
}

static int luaA_StartLoop(lua_State *L)
{
    checkargs("audio.StartLoop", 2);
    
    std::string start(Util::StrToLower(lua_tostring_view(L, 1)));
    std::string loop(Util::StrToLower(lua_tostring_view(L, 2)));
    
    lua_pop(L, nargs);
    
    Sound::PlaySFXStartLoop(start, loop);
    
    return 0;
}


static int luaA_PlayMusic(lua_State *L)
{
    checkargs("audio.PlayMusic", 1);
    
    std::string music(Util::StrToLower(lua_tostring_view(L, 1)));
    
    lua_pop(L, nargs);
    
    Sound::PlayMusic(music);
    
    return 0;
}

static int luaA_FadeMusic(lua_State *L)
{
    checkargs("audio.FadeMusic", 1);
    
    Sound::FadeMusic(lua_tointeger(L, 1));
    
    lua_pop(L, nargs);
    return 0;
}

static const luaL_Reg game_funcs[]
{
    {"MsTime", luaG_MsTime},
    {"GetText", luaG_GetText},
    {"GetInitText", luaG_GetInitText},
    {"IsLoadingSave", luaG_IsLoadingSave},
    {"HasSave", luaG_HasSave},
    {"ToGame", luaG_ToGame},
    {"Log", luaG_Log},
    {"ClearSave", luaG_ClearSave},
    {"Close", luaG_Close},
    {"NumInitTexts", NULL},
    {"NumRecoveryInitTexts", NULL},
    {NULL, NULL}
};

static const luaL_Reg config_funcs[]
{
    {"GetStringOr", luaC_GetStringOr},
    {"SetString", luaC_SetString},
    {NULL, NULL}
};

static const luaL_Reg audio_funcs[]
{
    {"PlaySample", luaA_PlaySample},
    {"StartLoop", luaA_StartLoop},
    {"PlayMusic", luaA_PlayMusic},
    {"FadeMusic", luaA_FadeMusic},
    {"IsSamplePlaying", luaA_IsSamplePlaying},
    {NULL, NULL}
};

LUAMOD_API int luaopen_game(lua_State *L)
{
    luaL_newlib(L, game_funcs);
    
    lua_pushinteger(L, initText.size());
    lua_setfield(L, -2, "NumInitTexts");
    lua_pushinteger(L, numRecoveryTexts);
    lua_setfield(L, -2, "NumRecoveryInitTexts");
    
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
