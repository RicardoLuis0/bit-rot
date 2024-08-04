#include "Game.h"
#include "Common.h"
#include "Renderer.h"
#include "Input.h"
#include "Config.h"
#include "SDL2Util.h"
#include "Menu.h"
#include "SaveData.h"

#include "Scripting/Lua/lua.h"
#include "Scripting/Lua/lualib.h"
#include "Scripting/Lua/lauxlib.h"

extern bool RunGame;
extern int currentScreen;

void Game::EndResponder(SDL_Event *e)
{
    if(e->type == SDL_KEYDOWN && e->key == SDLK_ESCAPE)
    {
        RunGame = false;
    }
}

void Game::TickEnd()
{
    Renderer::DrawClear();
    
    Menu::DrawTextBox(0, texts["EndMessage"], texts["EndMessage2"], true);
}

static lua_State * IntroVM = nullptr;
/*
void Game::EndIntro()
{
    lua_close(IntroVM);
    IntroVM = nullptr;
}
*/

void Game::ToIntro()
{
    currentScreen = 999;
    
    if(!IntroVM)
    {
        IntroVM = luaL_newstate();
        luaL_openlibs(IntroVM);
        if(luaL_loadfile(IntroVM, "GameData/intro.lua") != LUA_OK)
        {
            throw std::runtime_error(lua_tostring(IntroVM, -1));
        }
        
        if(lua_pcall(IntroVM, 0, 0, 0) != LUA_OK)
        {
            throw std::runtime_error(lua_tostring(IntroVM, -1));
        }
    }
    lua_State *  L = IntroVM;
    
    lua_getglobal(L, "init");
    
    if(!lua_isfunction(L, -1))
    {
        throw std::runtime_error("init missing or not a function in intro.lua");
    }
    
    if(lua_pcall(L, 0, 0, 0) != LUA_OK)
    {
        throw std::runtime_error(lua_tostring(L, -1));
    }
    
    lua_pop(L, lua_gettop(L));
    
    currentScreen = 3;
}

void Game::IntroResponder(SDL_Event *e)
{
    if(!IntroVM) return;
    
    if(e->type == SDL_KEYDOWN)
    {
        lua_State *  L = IntroVM;
        
        lua_getglobal(L, "responder");
        
        if(!lua_isfunction(L, -1))
        {
            throw std::runtime_error("responder missing or not a function in intro.lua");
        }
        
        lua_pushnumber(L, e->key.keysym.sym);
        
        
        if(lua_pcall(L, 1, 0, 0) != LUA_OK)
        {
            throw std::runtime_error(lua_tostring(L, -1));
        }
        
        lua_pop(L, lua_gettop(L));
    }
}

void Game::TickIntro()
{
    if(!IntroVM) return;
    
    Renderer::DrawClear();
    
    lua_State *  L = IntroVM;
    
    lua_getglobal(L, "tick");
    
    if(!lua_isfunction(L, -1))
    {
        throw std::runtime_error("tick missing or not a function in intro.lua");
    }
    
    if(lua_pcall(L, 0, 0, 0) != LUA_OK)
    {
        throw std::runtime_error(lua_tostring(L, -1));
    }
    
    lua_pop(L, lua_gettop(L));
}
