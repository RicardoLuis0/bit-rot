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

//#define NOLUA

#ifdef NOLUA

#define DEBUG_START_SCREEN 3
#define DEBUG_START_STAGE2 1
#define DEBUG_START_STAGE 0

int introStage = 0;
uint32_t introStartMs = 0;
uint32_t glitchStartMs = 0;

#define MEMORY_MB 16
#define MEMORY_MB_TXT "16"

uint32_t memAmountTarget = 1024 * MEMORY_MB;
uint32_t memIncrement = 64;
uint32_t memIncrementTimer = 25;


uint32_t lastIncrementMs = 0;
uint32_t memAmount = memIncrement;

template<size_t N = 80 * 40>
static constexpr FakeString<N> randFill(uint8_t min, uint8_t max, uint32_t seed = 123456)
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

uint32_t nextLineMs = 0;
uint32_t nextLine = 0;
uint32_t lineCountRecovery = 0;

bool skipText = false;

std::string message = "While rummaging through your childhood home's attic after your parents' passing, you found an old computer that you don't seem to have any recollection of.\n\n\n\nYou decided to dust it off, plug it in, and see if it works...";
std::string message2 = "Press any key to continue";

std::string end_message = "You've reached the end of this Build of BitRot, Congratulations, and Thanks for Playing!";
std::string end_message2 = "Press escape to close the game";

std::vector<std::string_view> messageLines = Util::SplitLines(message, 50);
std::vector<std::string_view> endMessageLines = Util::SplitLines(end_message, 50);

//auto randPrintErr = randFill(1, 254, 100);
FakeString<80*40> randPrintErr;

void Game::ToIntro()
{
    currentScreen = DEBUG_START_SCREEN;
    
    if(Game::GameIsSave || Config::getScriptStringOr("SawIntro1", "no") == "yes")
    {
        introStage = DEBUG_START_STAGE2;
        Audio::FadeMusic(500);
        Audio::StartFan();
        Audio::PlayMusic("lost");
    }
    else
    {
        introStage = DEBUG_START_STAGE;
        Audio::FadeMusic(500);
        Audio::PlayMusic("forest");
    }
}

void Game::IntroResponder(SDL_Event *e)
{
    if(introStage == 0 && e->type == SDL_KEYDOWN)
    {
        introStartMs = Util::MsTime();
        introStage = 1;
        Audio::StartFan();
        Audio::FadeMusic(1500);
    }
    else if(skipText && SaveData::HasSave() && introStage > 1 && e->type == SDL_KEYDOWN && e->key == SDLK_ESCAPE)
    {
        ToGame();
    }
}

static void DoIntroShared()
{
    switch(introStage)
    {
    case 0:
        if(!skipText)
        {
            Menu::DrawTextBox(0, texts["IntroMessage"], texts["IntroMessage2"], false);
            break;
        }
        else
        {
            introStage = 1;
        }
        [[fallthrough]];
    case 1:
        if((Util::MsTime() - introStartMs) >= 3000)
        {
            introStartMs = Util::MsTime();
            lastIncrementMs = introStartMs;
            introStage = 2;
            memAmount = memIncrement;
            Audio::Beep();
        }
        break;
    case 2:
    case 3:
        {
            if(memAmount == memAmountTarget && (Util::MsTime() - lastIncrementMs) >= 3500)
            {
                Renderer::HighRes();
                
                if(skipText && SaveData::HasSave())
                {
                    Renderer::DrawLineTextCentered(39, "Press ESCAPE to Skip");
                    Renderer::DrawFillLineProp(0, 39, CHAR_BLINK3, 80);
                }
            }
            else
            {
                Renderer::LowRes();
                
                if(skipText && SaveData::HasSave())
                {
                    Renderer::DrawLineTextCentered(24, "Press ESCAPE to Skip");
                    Renderer::DrawFillLineProp(0, 24, CHAR_BLINK3, 80);
                }
            }
            
            
            std::string target_msg = std::to_string(memAmountTarget) + " KB OK";
            std::string msg = std::to_string(memAmount) + " KB OK";
            Renderer::DrawLineText(1 + (target_msg.length() - msg.length()), 1, std::to_string(memAmount) + " KB OK");
            while((Util::MsTime() - lastIncrementMs) >= memIncrementTimer && memAmount < memAmountTarget)
            {
                lastIncrementMs += memIncrementTimer;
                memAmount += memIncrement;
            }
            
            if(memAmount == memAmountTarget)
            {
                Renderer::DrawLineTextFillProp(1, 2, "_", CHAR_BLINK2);
                if(introStage == 2)
                {
                    Audio::Beep();
                    introStage = 3;
                }
            }
            
            if(memAmount == memAmountTarget && (Util::MsTime() - lastIncrementMs) >= 4000)
            {
                introStartMs = Util::MsTime();
                nextLineMs = introStartMs;
                nextLine = 0;
                introStage = 4;
            }
        }
        break;
    }
}

static void DoIntro1()
{
    switch(introStage)
    {
    case 0:
    case 1:
    case 2:
    case 3:
        skipText = false;
        DoIntroShared();
        break;
    case 4:
    case 5:
        {
            while(Util::MsTime() >= nextLineMs && nextLine < (initText.size() - numRecoveryTexts))
            {
                nextLineMs += initText[nextLine].timer;
                if(initText[nextLine].beep)
                {
                    Audio::Beep();
                }
                nextLine++;
                
            }
            
            uint32_t start = 0;
            uint32_t offset = 1;
            uint32_t numErrors = 0;
            constexpr uint32_t errorMaxLine1 = 8;
            constexpr uint32_t errorMaxLine2 = 13;
            constexpr uint32_t errorMaxLine3 = 16;
            constexpr uint32_t errorDelayMs = 1000;
            
            if(introStage == 4)
            {
                if(nextLine < 38)
                {
                    start = 0;
                    offset = 2;
                    Renderer::DrawLineText(1, 1, std::to_string(memAmountTarget) + " KB OK");
                }
                else
                {
                    start = nextLine - 38;
                    offset = 1;
                }
            }
            else if(introStage == 5)
            {
                start = ((initText.size() - numRecoveryTexts) - 38);
                offset = 1;
                
                constexpr uint32_t errorsMs1 = 40;
                constexpr uint32_t errorsMs2 = 20;
                constexpr uint32_t errorsMs3 = 5;
                
                constexpr uint32_t errorMsLine1 = errorsMs1 * errorMaxLine1;
                constexpr uint32_t errorMsLine2 = errorsMs2 * errorMaxLine2;
                //constexpr uint32_t errorMsLine3 = errorsMs3 * errorMaxLine3;
                
                uint32_t time = Util::MsTime() - introStartMs;
                
                if(time < errorDelayMs)
                {
                    numErrors = 0;
                }
                else
                {
                    time -= errorDelayMs;
                    if(time <= errorMsLine1)
                    {
                        numErrors = time / errorsMs1;
                    }
                    else if(time <= (errorMsLine1 + errorMsLine2))
                    {
                        time -= errorMsLine1;
                        numErrors = (time / errorsMs2) + errorMaxLine1;
                        start++;
                    }
                    else
                    {
                        time -= errorMsLine1 + errorMsLine2;
                        numErrors = (time / errorsMs3) + errorMaxLine1 + errorMaxLine2;
                        
                        start += 1 + ((numErrors - (errorMaxLine1 + errorMaxLine2)) / errorMaxLine3);
                        
                        //if(start >= initText.size()) start = initText.size() - 1;// TODO remove after testing
                    }
                }
            }
            
            if(start < (initText.size() - numRecoveryTexts))
            {
                uint32_t n = nextLine - start;
                for(uint32_t i = 0; i < n; i++)
                {
                    Renderer::DrawLineText(1, i + offset, initText[start + i].text);
                    if(i == (n - 1) && nextLine < (initText.size() - numRecoveryTexts))
                    {
                        Renderer::DrawLineTextFillProp(1 + initText[start + i].text.length(), i + offset, ".", CHAR_BLINK1);
                    }
                }
            }
            else
            {
                introStartMs = Util::MsTime();
                glitchStartMs = introStartMs;
                introStage = 6;
                randPrintErr = randFill(1, 254, glitchStartMs);
            }
            
            if(introStage == 5 && numErrors > 0)
            {
                uint32_t offsetX = 29;
                uint32_t offsetY = offset + (nextLine - start);
                
                offsetY--;
                
                size_t n1 = std::min(numErrors, errorMaxLine1);
                for(size_t i = 0; i < n1; i++)
                {
                    Renderer::DrawLineText(offsetX, offsetY, "ERROR");
                    offsetX += 6;
                }
                
                uint32_t drawn = n1;
                
                if(numErrors > errorMaxLine1)
                {
                    offsetX = 0;
                    offsetY++;
                    
                    size_t n2 = std::min(numErrors - errorMaxLine1, errorMaxLine2);
                    for(size_t i = 0; i < n2; i++)
                    {
                        Renderer::DrawLineText(offsetX, offsetY, "ERROR");
                        offsetX += 6;
                    }
                    
                    drawn += n2;
                }
                if(numErrors > (errorMaxLine1 + errorMaxLine2))
                {
                    size_t n2 = errorMaxLine3;
                    while(n2 == errorMaxLine3)
                    {
                        offsetX = 0;
                        offsetY++;
                        n2 = std::min(numErrors - drawn, errorMaxLine3);
                        for(size_t i = 0; i < n2; i++)
                        {
                            Renderer::DrawLineText(offsetX, offsetY, "ERROR");
                            offsetX += 5;
                        }
                        drawn += n2;
                    }
                }
                
            }
            
            if(nextLine == (initText.size() - numRecoveryTexts) && introStage == 4)
            {
                introStartMs = Util::MsTime();
                introStage = 5;
                Audio::Error();
            }
        }
        if(introStage != 6) break;
        [[fallthrough]];
    case 6:
        Renderer::SetText(randPrintErr);
        if((Util::MsTime() - glitchStartMs) >= 100)
        {
            glitchStartMs = Util::MsTime();
            randPrintErr = randFill(1, 254, glitchStartMs);
        }
        
        if(Audio::ErrorPlaying())
        {
            break;
        }
        else
        {
            introStartMs = Util::MsTime();
            introStage = 7;
        }
        [[fallthrough]];
    case 7:
        Renderer::DrawClear(' ', CHAR_INVERT1);
        if((Util::MsTime() - introStartMs) >= 1000)
        {
            Config::setString("SawIntro1", "yes");
            RunGame = false;
            SaveData::Clear();
            //introStartMs = Util::MsTime();
            //introStage = 8;
        }
        break;
    }
}

void DoIntro2()
{
    switch(introStage)
    {
    case 0:
    case 1:
    case 2:
    case 3:
        skipText = true;
        DoIntroShared();
        break;
    case 4:
        if(Game::GameIsSave)
        {
            Renderer::DrawLineTextCentered(39, "Press ESCAPE to Skip");
            Renderer::DrawFillLineProp(0, 39, CHAR_BLINK3, 80);
        }
        
        Renderer::DrawLineText(1, 1, std::to_string(memAmountTarget) + " KB OK");
        Renderer::DrawLineTextCentered(3, "LAST BOOT FAILED");
        Renderer::DrawLineTextCentered(4, "ENTERING RECOVERY MODE");
        Renderer::DrawFillLineProp(0, 3, CHAR_INVERT2, 80);
        Renderer::DrawFillLineProp(42, 3, CHAR_INVERT2 | CHAR_BLINK2, 6);
        Renderer::DrawFillLineProp(0, 4, CHAR_INVERT2, 80);
        Renderer::DrawFillLineProp(38, 4, CHAR_INVERT2 | CHAR_BLINK2, 13);
        if((Util::MsTime() - introStartMs) >= 2000)
        {
            introStartMs = Util::MsTime();
            nextLine = 0;
            nextLineMs = introStartMs;
            introStage = 5;
            lineCountRecovery = 0;
            Audio::Beep();
        }
        break;
    case 5:
        {
            if(Game::GameIsSave)
            {
                Renderer::DrawLineTextCentered(39, "Press ESCAPE to Skip");
                Renderer::DrawFillLineProp(0, 39, CHAR_BLINK3, 80);
            }
            
            while(Util::MsTime() >= nextLineMs && nextLine < initText.size())
            {
                if(initText[nextLine].recovery)
                {
                    nextLineMs += initText[nextLine].timer;
                    if(initText[nextLine].beep)
                    {
                        Audio::Beep();
                    }
                    lineCountRecovery++;
                }
                nextLine++;
            }
            
            uint32_t start;
            uint32_t offset;
            
            if(lineCountRecovery < 35)
            {
                start = 0;
                offset = 5;
                Renderer::DrawLineText(1, 1, std::to_string(memAmountTarget) + " KB OK");
                Renderer::DrawLineTextCentered(3, "LAST BOOT FAILED");
                Renderer::DrawLineTextCentered(4, "ENTERING RECOVERY MODE");
                Renderer::DrawFillLineProp(0, 3, CHAR_INVERT2, 80);
                Renderer::DrawFillLineProp(0, 4, CHAR_INVERT2, 80);
            }
            else if(lineCountRecovery < 37)
            {
                start = 0;
                offset = 3 + (lineCountRecovery < 36);
                Renderer::DrawLineTextCentered(1 + (lineCountRecovery < 36), "LAST BOOT FAILED");
                Renderer::DrawLineTextCentered(2 + (lineCountRecovery < 36), "ENTERING RECOVERY MODE");
                Renderer::DrawFillLineProp(0, 1 + (lineCountRecovery < 36), CHAR_INVERT2, 80);
                Renderer::DrawFillLineProp(0, 2 + (lineCountRecovery < 36), CHAR_INVERT2, 80);
            }
            else if(lineCountRecovery < 38)
            {
                start = 0;
                offset = 2;
                Renderer::DrawLineTextCentered(1, "ENTERING RECOVERY MODE");
                Renderer::DrawFillLineProp(0, 1, CHAR_INVERT2, 80);
            }
            else
            {
                start = lineCountRecovery - 38;
                offset = 1;
            }
            
            uint32_t n = nextLine - start;
            uint32_t j = 0;
            for(uint32_t i = 0; i < n; i++)
            {
                if(initText[start + i].recovery)
                {
                    Renderer::DrawLineText(1, j + offset, initText[start + i].text);
                    if(i == (n - 1))
                    {
                        Renderer::DrawLineTextFillProp(1 + initText[start + i].text.length(), j + offset, ".", CHAR_BLINK1);
                    }
                    j++;
                }
            }
            
            if(nextLine == initText.size() && Util::MsTime() >= nextLineMs)
            {
                Game::ToGame();
            }
        }
    case 6:
        break;
    }
}

void Game::TickIntro()
{
    Renderer::DrawClear();
    if(!(Game::GameIsSave || Config::getScriptStringOr("SawIntro1", "no") == "yes"))
    {
        DoIntro1();
    }
    else
    {
        DoIntro2();
    }
    //TODO
}

#else

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

#endif
