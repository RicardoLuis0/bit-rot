#include <stdexcept>
#include <cassert>
#include <map>

#include "Config.h"
#include "Common.h"
#include "Exceptions.h"
#include "Log.h"
#include "Font.h"
#include "SDL2Util.h"

#include "Renderer/Internal.h"
#include "Renderer/GLQuad.h"
#include "Renderer/GLFrameBuffer.h"

using namespace Renderer;
using namespace Renderer::Internal;


constexpr uint32_t max_screen_width = 80;
constexpr uint32_t max_screen_height = 40;


TextBuffer Renderer::MenuText;
TextBuffer Renderer::GameText;

TextBuffer *Renderer::CurrentBuffer;

__thread char buf[1024];

SDL_Window * win;
SDL_GLContext ctx;

uint64_t baseTime;

GLProgram phosphorProgram;
GLProgram crtProgram;
GLProgram bloomProgram;
GLProgram textDrawerMenu;
GLProgram textDrawerGame;
GLProgram blurProgram;

GLQuad mainArea;
GLQuad textAreaMenu;
GLQuad textAreaGame;
GLQuad textAreaGameMenu;

GLTexture fontTexture;
GLUniformBuffer textBufferMenu;
GLUniformBuffer textBufferGame;
GLFrameBuffer textFrameBuffer;
GLFrameBuffer textFrameBuffer2;
GLFrameBuffer textFrameBuffer3;
GLFrameBuffer textFrameBuffer4;
GLFrameBuffer frameBuffer;

constexpr size_t numPhosphorBuffers = 10;

size_t phosphorBufferIndex = 0;

GLTexture phosphorBuffers[numPhosphorBuffers];

TextInfo * textBufferDataMenu;
TextInfo * textBufferDataGame;

bool window_ok = false;

namespace Renderer::Internal
{
    uint32_t window_width = 1280;
    uint32_t window_height = 960;
}

float textColors[uint8_t(ETextColor::COUNT)][4]
{
    {1.0, 0.7, 0.2, 1.0}, // YELLOW
    {1.0, 0.4, 0.1, 1.0}, // AMBER
    {1.0, 0.2, 0.2, 1.0}, // RED
    {0.2, 1.0, 0.2, 1.0}, // GREEN
    {1.0, 1.0, 1.0, 1.0}, // WHITE
};

const char * textColorNames[uint8_t(ETextColor::COUNT)]
{
    "Yellow",
    "Amber",
    "Red",
    "Green",
    "White"
};

ETextColor currentTextColor = ETextColor::YELLOW;

bool adaptiveOk = true;

void Renderer::SetVSync(std::string_view VSync)
{
    if(VSync == "Off")
    {
        SDL_GL_SetSwapInterval(0);
        LogDebug("VSync Disabled");
    }
    else if(VSync == "Adaptive")
    {
        if(!adaptiveOk || SDL_GL_SetSwapInterval(-1))
        {
            LogDebug("Failed to enable Adaptive VSync: " + errMsg());
            
            if(SDL_GL_SetSwapInterval(1))
            {
                VSync = "Off";
                SDL_GL_SetSwapInterval(0);
                LogWarn("Failed to enable VSync: " + errMsg());
            }
            else
            {
                VSync = "On";
                LogDebug("VSync Enabled");
            }
        }
        else
        {
            LogDebug("Adaptive VSync Enabled");
        }
    }
    else if(VSync == "On")
    {
        if(SDL_GL_SetSwapInterval(1))
        {
            VSync = "Off";
            LogWarn("Failed to enable VSync: " + errMsg());
        }
        else
        {
            LogDebug("VSync Enabled");
        }
    }
    else
    {
        SetVSync("Off");
        return;
    }
    Config::setString("VSync", VSync);
}

bool firstCompile = true;

constexpr int fontTextureU = 0;
constexpr int backgroundTextureU = 1;
constexpr int useBackgroundTextureU = 2;
constexpr int transparentKeyU = 3;
constexpr int timeU = 7;
constexpr int textColorU = 8;
constexpr int bloomStrengthU = 3;
constexpr int crtCurveU = 5;
constexpr int crtScanlinesU = 6;
constexpr int crtCAU = 7;
constexpr int crtVignetteU = 8;

void Renderer::UpdateBloomStrength()
{
    bloomProgram.setFloat(bloomStrengthU, sqrt(Config::getIntOr("BloomStrength", 100) / 100.0) * 0.85);
}

void Renderer::UpdateCrt()
{
    crtProgram.setFloat(crtCurveU, sqrt(Config::getIntOr("CrtCurve", 100) / 100.0));
    crtProgram.setInt(crtScanlinesU, Config::getIntOr("CrtScanlinesEnabled", 1));
    crtProgram.setInt(crtCAU, Config::getIntOr("CrtCAEnabled", 1));
    crtProgram.setInt(crtVignetteU, Config::getIntOr("CrtVignetteEnabled", 0));
}


bool firstRender = true;

constexpr float DefaultPhosphorStrength = 0.5f;

void Renderer::PhosphorEnabled(bool yes)
{
    if(yes)
    {
        phosphorProgram.setFloat(1, DefaultPhosphorStrength);
        firstRender = true;
    }
    else
    {
        phosphorProgram.setFloat(1, 0.0);
    }
}

void Renderer::Compile()
{
    firstRender = true;
    
    vertexShaderCache.clear();
    fragShaderCache.clear();
    
    phosphorProgram.CompileAndLink("phosphor", "vertex.glsl", "phosphor.glsl");
    crtProgram.CompileAndLink("crt", "vertex.glsl", "crt.glsl");
    bloomProgram.CompileAndLink("bloom", "vertex.glsl", "bloom.glsl");
    textDrawerMenu.CompileAndLink("textDrawerMenu", "vertex.glsl", "text.glsl");
    textDrawerGame.CompileAndLink("textDrawerGame", "vertex.glsl", "text.glsl");
    blurProgram.CompileAndLink("blur", "vertex.glsl", "blur.glsl");
    
    textDrawerMenu.setInt(fontTextureU, 0);
    textDrawerGame.setInt(fontTextureU, 0);
    textDrawerMenu.setInt(backgroundTextureU, 1);
    textDrawerGame.setInt(backgroundTextureU, 1);
    textDrawerMenu.setInt(useBackgroundTextureU, 0);
    textDrawerGame.setInt(useBackgroundTextureU, 0);
    textDrawerMenu.setFloat(transparentKeyU, 1.0, 0.0, 1.0, 1.0);
    textDrawerGame.setFloat(transparentKeyU, 1.0, 0.0, 1.0, 1.0);
    
    phosphorProgram.setInt(0, 0);
    phosphorProgram.setFloat(1, Config::getIntOr("PhosphorEnabled", 1) ? DefaultPhosphorStrength : 0.0);
    
    crtProgram.setInt(1, window_width, window_height);
    crtProgram.setInt(0, 0);
    crtProgram.setInt(3, 8 * max_screen_width, 8 *max_screen_height); // fake out 8-width chars for CRT shader, looks bad otherwise for high-res fonts
    UpdateCrt();
    
    SetTextColor(Config::getEnumOr("TextColor", textColorNames, ETextColor::AMBER));
    
    bloomProgram.setInt(0, 0);
    bloomProgram.setInt(1, window_width, window_height);
    UpdateBloomStrength();
    
    LogDebug(firstCompile ? "Shaders Compiled" : "Shaders Recompiled");
    
    {
        
        TextInfo tmp2 = {};
        
        tmp2.screen_width = max_screen_width;
        tmp2.screen_height = max_screen_height;
        
        uint32_t w, h;
        
        auto &fnt = Font::getSelectedFont(w, h, tmp2.char_height, tmp2.char_width, tmp2.font_width, tmp2.font_height);
        
        fontTexture.LoadRGBA8(fnt.data(), w, h);
        
        LogDebug(firstCompile ? "Text Font Loaded" : "Text Font Reloaded");
        
        textBufferMenu.Init(&tmp2);
        textBufferGame.Init(&tmp2);
        
        textBufferDataMenu = reinterpret_cast<TextInfo*>(glMapNamedBufferRange(textBufferMenu.index, 0, sizeof(TextInfo), GL_MAP_WRITE_BIT | GL_MAP_READ_BIT));
        textBufferDataGame = reinterpret_cast<TextInfo*>(glMapNamedBufferRange(textBufferGame.index, 0, sizeof(TextInfo), GL_MAP_WRITE_BIT | GL_MAP_READ_BIT));
        
        MenuText.textBufferData = textBufferDataMenu;
        GameText.textBufferData = textBufferDataGame;
        
        glCheckErrors();
        SetTextColor(Config::getEnumOr("TextColor", textColorNames, ETextColor::AMBER));
        
        glCheckErrorsDebug();
        
        LogDebug(firstCompile ? "Text Info Loaded" : "Text Info Reloaded");
    }
    
    textFrameBuffer.Init(textBufferDataMenu->char_width * max_screen_width, textBufferDataMenu->char_height * max_screen_height);
    textFrameBuffer2.Init(textBufferDataMenu->char_width * max_screen_width, textBufferDataMenu->char_height * max_screen_height);
    textFrameBuffer3.Init(textBufferDataMenu->char_width * max_screen_width, textBufferDataMenu->char_height * max_screen_height);
    textFrameBuffer4.Init(textBufferDataMenu->char_width * max_screen_width, textBufferDataMenu->char_height * max_screen_height);
    frameBuffer.Init(window_width, window_height);
    
    blurProgram.setInt(1, textBufferDataMenu->char_width * max_screen_width, textBufferDataMenu->char_height * max_screen_height);
    
    mainArea.Gen(std::array { &crtProgram, &bloomProgram } ,-1.0, -1.0, 1.0, 1.0);
    
    textAreaMenu.Gen(std::array { &textDrawerMenu, &phosphorProgram } , -1.0, -1.0, 1.0, 1.0);
    textAreaGame.Gen(std::array { &textDrawerGame, &phosphorProgram } , -1.0, -1.0, 1.0, 1.0);
    textAreaGameMenu.Gen(std::array { &textDrawerGame, &blurProgram, &textDrawerMenu, &phosphorProgram } , -1.0, -1.0, 1.0, 1.0);
    
    LogDebug(firstCompile ? "Geometry Generated" : "Geometry Regenerated");
    
    mainArea.addTexture(std::array { &textFrameBuffer4.colorTexture, &frameBuffer.colorTexture });
    
    textAreaMenu.addUBO(&textBufferMenu);
    textAreaMenu.addTexture(std::array { &fontTexture, &textFrameBuffer3.colorTexture });
    
    textAreaGame.addUBO(&textBufferGame);
    textAreaGame.addTexture(std::array { &fontTexture, &textFrameBuffer3.colorTexture });
    
    textAreaGameMenu.addUBO(std::array { &textBufferGame, (GLUniformBuffer*)nullptr, &textBufferMenu});
    textAreaGameMenu.addTexture(std::array { &fontTexture, &textFrameBuffer.colorTexture, &fontTexture, &textFrameBuffer3.colorTexture });
    
    for(size_t i = 0; i < numPhosphorBuffers; i++)
    {
        phosphorBuffers[i].InitNew(textBufferDataMenu->char_width * max_screen_width, textBufferDataMenu->char_height * max_screen_height);
        textAreaMenu.addTexture(std::array { (GLTexture*)nullptr, phosphorBuffers + i });
        textAreaGame.addTexture(std::array { (GLTexture*)nullptr, phosphorBuffers + i });
        textAreaGameMenu.addTexture(std::array { (GLTexture*)nullptr, (GLTexture*)nullptr, i == 0 ? &textFrameBuffer2.colorTexture : (GLTexture*)nullptr, phosphorBuffers + i });
    }
    
    LogDebug(firstCompile ? "Framebuffer Generated" : "Framebuffer Regenerated");
    
    firstCompile = false;
}

void Renderer::Init()
{
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        throw FatalError(errMsg());
    }
    
    LogDebug("SDL Initialized");
    
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 5 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
    
    LogDebug("OpenGL version set to 4.5 Core");
    
    if(!(
            win = SDL_CreateWindow("Bit Rot", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE)
    )) {
        throw FatalError(errMsg());
    }
    
    LogDebug("SDL Window Created");
    
    if(!(
            ctx = SDL_GL_CreateContext(win)
    )) {
        throw FatalError(errMsg());
    }
    
    LogDebug("SDL GL Context Created");
    
    glewExperimental = true;
    if(auto err = glewInit(); err != GLEW_OK)
    {
        throw FatalError(reinterpret_cast<const char *>(glewGetErrorString(err)));
    }
    SetVSync(Config::getStringOr("VSync", "Adaptive"));
    
    
    LogDebug("GLEW Initialized");
    
    
    
    Compile();
    
    
    
    glCheckErrors();
    
    window_ok = true;
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    LogDebug("Window Cleared");
    
    SDL_ShowWindow(win);
    SDL_MaximizeWindow(win);
    SDL_GL_SwapWindow(win);
    
    baseTime = Util::MsTime();
    
    LogDebug("Window Shown");
}

void Renderer::TextBuffer::SetText(std::string_view newText)
{
    size_t bufSiz = 80 * 40;
    size_t n = std::min(bufSiz, newText.size());
    size_t n2 = bufSiz - n;
    
    memcpy(textBufferData->chars, newText.data(), n);
    
    if(n2 > 0)
    {
        memset(textBufferData->chars + n, 0, n2);
    }
    
    memset(textBufferData->char_properties, 0, bufSiz);
}


void Renderer::TextBuffer::LowRes()
{
    textBufferData->screen_width = 40;
    textBufferData->screen_height = 25;
}

void Renderer::TextBuffer::HighRes()
{
    textBufferData->screen_width = 80;
    textBufferData->screen_height = 40;
}

void Renderer::TextBuffer::SetText(std::string_view newText, std::span<const uint8_t> properties)
{
    size_t bufSiz = max_screen_width * max_screen_height;
    {
        size_t n = std::min(bufSiz, newText.size());
        size_t n2 = bufSiz - n;
        memcpy(textBufferData->chars, newText.data(), n);
        if(n2 > 0)
        {
            memset(textBufferData->chars + n, 0, n2);
        }
    }
    {
        size_t n = std::min(bufSiz, properties.size());
        size_t n2 = bufSiz - n;
        memcpy(textBufferData->char_properties, properties.data(), n);
        if(n2 > 0)
        {
            memset(textBufferData->char_properties + n, 0, n2);
        }
    }
}

void Renderer::UpdateResolution(uint32_t width, uint32_t height)
{
    window_width = width;
    window_height = height;
    
    calcWidthHeight(width, height);
    
    crtProgram.setInt(1, width, height);
    bloomProgram.setInt(1, width, height);
    frameBuffer.Resize(width, height);
}

bool skipNextFrame = false;

static void ReloadFont()
{
    uint32_t w, h;
    
    uint32_t oldw = textBufferDataMenu->char_width;
    uint32_t oldh = textBufferDataMenu->char_height;
    
    auto &fnt = Font::getSelectedFont(w, h, textBufferDataMenu->char_width, textBufferDataMenu->char_height, textBufferDataMenu->font_width, textBufferDataMenu->font_height);
    
    firstRender = true;
    
    if(oldw != textBufferDataMenu->char_width || oldh != textBufferDataMenu->char_height)
    {
        textBufferDataGame->char_width = textBufferDataMenu->char_width;
        textBufferDataGame->char_height = textBufferDataMenu->char_height;
        textBufferDataGame->font_width = textBufferDataMenu->font_width;
        textBufferDataGame->font_height = textBufferDataMenu->font_height;
        
        textFrameBuffer.Resize(textBufferDataMenu->char_width * max_screen_width, textBufferDataMenu->char_height * max_screen_height);
        textFrameBuffer2.Resize(textBufferDataMenu->char_width * max_screen_width, textBufferDataMenu->char_height * max_screen_height);
        textFrameBuffer3.Resize(textBufferDataMenu->char_width * max_screen_width, textBufferDataMenu->char_height * max_screen_height);
        textFrameBuffer4.Resize(textBufferDataMenu->char_width * max_screen_width, textBufferDataMenu->char_height * max_screen_height);
        
        //crtProgram.setInt(3, textBufferDataMenu->char_width * max_screen_width, textBufferDataMenu->char_height * max_screen_height); // fake out 8-width chars for CRT shader, looks bad otherwise for high-res fonts
        
        for(size_t i = 0; i < numPhosphorBuffers; i++)
        {
            phosphorBuffers[i].InitNew(textBufferDataMenu->char_width * max_screen_width, textBufferDataMenu->char_height * max_screen_height);
        }
    }
    
    fontTexture.UpdateRGBA8(fnt.data(), w, h);
    
    skipNextFrame = true;
}

void Renderer::SetFont(uint32_t index)
{
    Font::setFont(index);
    ReloadFont();
}

void Renderer::SetFont(std::string_view name)
{
    Font::setFont(name);
    ReloadFont();
}

void Renderer::CycleFont()
{
    Font::setFont(Font::curFontIndex() + 1);
    ReloadFont();
}

void Renderer::CycleFontDown()
{
    int32_t i = Font::curFontIndex();
    Font::setFont(i - 1);
    ReloadFont();
}

void Renderer::ToggleWireframe()
{
    static bool t;
    
    if(t)
    {
        LogDebug("Wireframe Disabled");
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    else
    {
        LogDebug("Wireframe Enabled");
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    
    t = !t;
}

void Renderer::ResetTimer()
{
    baseTime = Util::MsTime();
}

bool Renderer::DrawMenu = true;
bool Renderer::DrawGame = false;

void Renderer::Render()
{
    uint32_t w = textBufferDataMenu->char_width * max_screen_width;
    uint32_t h = textBufferDataMenu->char_height * max_screen_height;
    
    
    textBufferDataMenu = nullptr;
    textBufferDataGame = nullptr;
    //flush text
    glUnmapNamedBuffer(textBufferMenu.index);
    glUnmapNamedBuffer(textBufferGame.index);
    
    if(skipNextFrame)
    {
        skipNextFrame = false;
    }
    else
    {
        glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT | GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT);
        
        if(DrawMenu)
        {
            textDrawerMenu.setUInt(timeU, Util::MsTime() - baseTime);
        }
        else
        {
            textDrawerGame.setUInt(timeU, Util::MsTime() - baseTime);
        }
        
        if(!firstRender)
        {
            phosphorBufferIndex = (phosphorBufferIndex + 1) % numPhosphorBuffers;
            glCopyImageSubData(textFrameBuffer3.colorTexture.index, GL_TEXTURE_2D, 0, 0, 0, 0, phosphorBuffers[phosphorBufferIndex].index, GL_TEXTURE_2D, 0, 0, 0, 0, w, h, 1);
            
            for(unsigned i = 0; i < numPhosphorBuffers; i++)
            {
                phosphorProgram.setInt(2 + i,1 + ((phosphorBufferIndex + i) % numPhosphorBuffers));
            }
            
            glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT); // probably not necessary, but...
        }
        
        if(firstRender)
        { // fill phosphor buffer
            phosphorBufferIndex = 0;
            int n = (numPhosphorBuffers * 2);
            for(int i = 0; i < n; i++)
            {
                //only supported in menus
                textDrawerMenu.setInt(useBackgroundTextureU, 0);
                textAreaMenu.Render(std::array {&textFrameBuffer3, &textFrameBuffer4});
                phosphorBufferIndex = (i % numPhosphorBuffers);
                glCopyImageSubData(textFrameBuffer3.colorTexture.index, GL_TEXTURE_2D, 0, 0, 0, 0, phosphorBuffers[phosphorBufferIndex].index, GL_TEXTURE_2D, 0, 0, 0, 0, w, h, 1);
            }
            
            firstRender = false;
        }
        
        if(DrawMenu && DrawGame)
        {
            textDrawerMenu.setInt(useBackgroundTextureU, 1);
            textAreaGameMenu.Render(std::array {&textFrameBuffer, &textFrameBuffer2, &textFrameBuffer3, &textFrameBuffer4});
            mainArea.Render(std::array {&frameBuffer, (GLFrameBuffer*)nullptr});
        }
        else if(DrawGame)
        {
            textAreaGame.Render(std::array {&textFrameBuffer3, &textFrameBuffer4});
            mainArea.Render(std::array {&frameBuffer, (GLFrameBuffer*)nullptr});
        }
        else // if(drawMenu)
        {
            textDrawerMenu.setInt(useBackgroundTextureU, 0);
            textAreaMenu.Render(std::array {&textFrameBuffer3, &textFrameBuffer4});
            mainArea.Render(std::array {&frameBuffer, (GLFrameBuffer*)nullptr});
        }
        
        glCheckErrors();
    }
    
    SDL_GL_SwapWindow(win);
    
    textBufferDataMenu = reinterpret_cast<TextInfo*>(glMapNamedBufferRange(textBufferMenu.index, 0, sizeof(TextInfo), GL_MAP_WRITE_BIT | GL_MAP_READ_BIT));
    textBufferDataGame = reinterpret_cast<TextInfo*>(glMapNamedBufferRange(textBufferGame.index, 0, sizeof(TextInfo), GL_MAP_WRITE_BIT | GL_MAP_READ_BIT));
    
    MenuText.textBufferData = textBufferDataMenu;
    GameText.textBufferData = textBufferDataGame;
}

void Renderer::Quit()
{
    window_ok = false;
    if(win)
    {
        LogDebug("Window Destroyed");
        SDL_DestroyWindow(win);
    }
    
    LogDebug("SDL Quitted");
    SDL_Quit();
}

void Renderer::CycleTextColor()
{
    SetTextColor(ETextColor((uint8_t(currentTextColor) + 1) % uint8_t(ETextColor::COUNT)));
}

void Renderer::CycleTextColorDown()
{
    SetTextColor(ETextColor((uint8_t((uint8_t(currentTextColor) == 0) ? ETextColor::COUNT : currentTextColor) - 1)));
}


void Renderer::CycleVSync()
{
    std::string_view vsync = Config::mustGetString("VSync");
    if(vsync == "Off")
    {
        SetVSync(adaptiveOk ? "Adaptive" : "On");
    }
    else if(vsync == "Adaptive")
    {
        SetVSync("On");
    }
    else // On
    {
        SetVSync("Off");
    }
}

void Renderer::CycleVSyncDown()
{
    std::string_view vsync = Config::mustGetString("VSync");
    if(vsync == "Off")
    {
        SetVSync("On");
    }
    else if(vsync == "Adaptive")
    {
        SetVSync("Off");
    }
    else // On
    {
        SetVSync(adaptiveOk ? "Adaptive" : "Off");
    }
}

std::string_view Renderer::GetTextColorName()
{
    return textColorNames[uint8_t(currentTextColor)];
}

void Renderer::SetTextColor(ETextColor color)
{
    assert(uint8_t(color) < uint8_t(ETextColor::COUNT));
    
    LogDebug("Setting text color to "_s + textColorNames[uint8_t(color)]);
    
    currentTextColor = color;
    float *c = textColors[uint8_t(color)];
    if(textDrawerMenu.program) textDrawerMenu.setFloat(textColorU, c[0], c[1], c[2], c[3]);
    if(textDrawerGame.program) textDrawerGame.setFloat(textColorU, c[0], c[1], c[2], c[3]);
    Config::setEnum("TextColor", textColorNames, color);
}

void Util::ShowFatalError(const std::string &title,const std::string &msg)
{
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title.c_str(), msg.c_str(), window_ok ? win : nullptr);
}


void Renderer::TextBuffer::DrawLineText(uint32_t x, uint32_t y, std::string_view newText, uint32_t width)
{
    if(x >= textBufferData->screen_width || y >= textBufferData->screen_height)
    {
        return;
    }
    size_t m = size_t(textBufferData->screen_width - x);
    
    size_t n = std::min(width != 0 ? std::min<size_t>(newText.size(), width) : newText.size(), m);
    
    size_t offset = x + (y * textBufferData->screen_width);
    
    memcpy(textBufferData->chars + offset, newText.data(), n);
    
    if(m > n && width > n)
    {
        size_t n2 = std::min(width - n, m - n);
        
        memset(textBufferData->chars + offset + n, 0, n2);
        
        n += n2;
    }
}

void Renderer::TextBuffer::DrawLineTextFillProp(uint32_t x, uint32_t y, std::string_view newText, uint8_t newProperty, uint32_t width)
{
    if(x >= textBufferData->screen_width || y >= textBufferData->screen_height)
    {
        return;
    }
    size_t m = size_t(textBufferData->screen_width - x);
    
    size_t n = std::min(width != 0 ? std::min<size_t>(newText.size(), width) : newText.size(), m);
    
    size_t offset = x + (y * textBufferData->screen_width);
    
    memcpy(textBufferData->chars + offset, newText.data(), n);
    
    if(m > n && width > n)
    {
        size_t n2 = std::min(width - n, m - n);
        
        memset(textBufferData->chars + offset + n, 0, n2);
        
        n += n2;
    }
    
    memset(textBufferData->char_properties + offset, newProperty, n);
}

void Renderer::TextBuffer::DrawLineProp(uint32_t x, uint32_t y, std::span<const uint8_t> newProperties, uint32_t width)
{
    if(x >= textBufferData->screen_width || y >= textBufferData->screen_height)
    {
        return;
    }
    size_t m = size_t(textBufferData->screen_width - x);
    
    size_t n = std::min(width != 0 ? std::min<size_t>(newProperties.size(), width) : newProperties.size(), m);
    
    size_t offset = x + (y * textBufferData->screen_width);
    
    memcpy(textBufferData->char_properties + offset, newProperties.data(), n);
    
    if(m > n && width > n)
    {
        size_t n2 = std::min(width - n, m - n);
        
        memset(textBufferData->char_properties + offset + n, 0, n2);
        
        n += n2;
    }
}

void Renderer::TextBuffer::DrawFillLineProp(uint32_t x, uint32_t y, uint8_t newProperty, uint32_t width)
{
    if(x >= textBufferData->screen_width || y >= textBufferData->screen_height)
    {
        return;
    }
    size_t m = size_t(textBufferData->screen_width - x);
    
    size_t n = std::min<size_t>(width, m);
    
    size_t offset = x + (y * textBufferData->screen_width);
    
    memset(textBufferData->char_properties + offset, newProperty, n);
}

void Renderer::TextBuffer::DrawFillLineText(uint32_t x, uint32_t y, char newText, uint32_t width)
{
    if(x >= textBufferData->screen_width || y >= textBufferData->screen_height)
    {
        return;
    }
    size_t m = size_t(textBufferData->screen_width - x);
    
    size_t n = std::min<size_t>(width, m);
    
    size_t offset = x + (y * textBufferData->screen_width);
    
    memset(textBufferData->chars + offset, newText, n);
}

void Renderer::TextBuffer::DrawFillLineTextProp(uint32_t x, uint32_t y, char newText, uint8_t newProperty, uint32_t width)
{
    if(x >= textBufferData->screen_width || y >= textBufferData->screen_height)
    {
        return;
    }
    size_t m = size_t(textBufferData->screen_width - x);
    
    size_t n = std::min<size_t>(width, m);
    
    size_t offset = x + (y * textBufferData->screen_width);
    
    memset(textBufferData->chars + offset, newText, n);
    memset(textBufferData->char_properties + offset, newProperty, n);
}

void Renderer::TextBuffer::DrawChar(uint32_t x, uint32_t y, char newChar, uint8_t newProperty)
{
    if(x >= textBufferData->screen_width || y >= textBufferData->screen_height)
    {
        return;
    }
    
    size_t offset = x + (y * textBufferData->screen_width);
    
    textBufferData->chars[offset] = newChar;
    textBufferData->char_properties[offset] = newProperty;
}

template<bool text = true, bool props = true>
static void DrawClear(TextInfo * textBufferData, uint32_t x, uint32_t y, uint32_t width)
{
    if(x >= textBufferData->screen_width || y >= textBufferData->screen_height)
    {
        return;
    }
    size_t m = size_t(textBufferData->screen_width - x);
    
    size_t n = std::min<size_t>(width, m);
    
    size_t offset = x + (y * textBufferData->screen_width);
    
    if constexpr(text) memset(textBufferData->chars + offset, 0, n);
    if constexpr(props) memset(textBufferData->char_properties + offset, 0, n);
}

void Renderer::TextBuffer::DrawClear(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    for(uint32_t i = 0; i < height; i++)
    {
        ::DrawClear(textBufferData, x, y + i, width);
    }
}

void Renderer::TextBuffer::DrawClearText(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    for(uint32_t i = 0; i < height; i++)
    {
        ::DrawClear<true, false>(textBufferData, x, y + i, width);
    }
}

void Renderer::TextBuffer::DrawClearProps(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    for(uint32_t i = 0; i < height; i++)
    {
        ::DrawClear<false, true>(textBufferData, x, y + i, width);
    }
}

void Renderer::TextBuffer::DrawClear()
{
    size_t bufSiz = max_screen_width * max_screen_height;
    
    memset(textBufferData->chars, 0, bufSiz * 2);
}

void Renderer::TextBuffer::DrawClear(uint8_t text_char, uint8_t prop)
{
    size_t bufSiz = max_screen_width * max_screen_height;
    
    memset(textBufferData->chars, text_char, bufSiz);
    memset(textBufferData->char_properties, prop, bufSiz);
}

void Renderer::TextBuffer::DrawLineTextCentered(uint32_t y, std::string_view newText, char prop)
{
    assert(std::size(newText) <= textBufferData->screen_width);
    uint32_t x = (textBufferData->screen_width - std::size(newText)) / 2;
    if(prop)
    {
        DrawLineTextFillProp(x, y, newText, prop);
    }
    else
    {
        DrawLineText(x, y, newText);
    }
}
