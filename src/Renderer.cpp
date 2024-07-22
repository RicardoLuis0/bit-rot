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

__thread char buf[1024];

SDL_Window * win;
SDL_GLContext ctx;

uint64_t baseTime;

struct TextInfo
{
    uint32_t font_width;
    uint32_t font_height;
    uint32_t char_width;
    uint32_t char_height;
    uint32_t screen_width;
    uint32_t screen_height;
    alignas(16) char chars[3200];
    alignas(16) uint8_t char_properties[3200];
};
static_assert(sizeof(TextInfo::chars) == 80 * 40);
GLProgram phosphorProgram;
GLProgram crtProgram;
GLProgram bloomProgram;
GLProgram textDrawer;

GLQuad mainArea;
GLQuad textArea;

GLTexture fontTexture;
GLUniformBuffer textBuffer;
GLFrameBuffer textFrameBuffer;
GLFrameBuffer textFrameBuffer2;
GLFrameBuffer frameBuffer;

constexpr size_t numPhosphorBuffers = 10;

size_t phosphorBufferIndex = 0;

GLTexture phosphorBuffers[numPhosphorBuffers];

TextInfo * textBufferData;

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



void Renderer::Compile()
{
    vertexShaderCache.clear();
    fragShaderCache.clear();
    
    phosphorProgram.CompileAndLink("phosphor", "vertex.glsl", "phosphor.glsl");
    crtProgram.CompileAndLink("crt", "vertex.glsl", "crt.glsl");
    bloomProgram.CompileAndLink("bloom", "vertex.glsl", "bloom.glsl");
    textDrawer.CompileAndLink("textDrawer", "vertex.glsl", "text.glsl");
    
    textDrawer.setInt(0, 0);
    textDrawer.setFloat(1, 1.0, 0.0, 1.0, 1.0);
    
    phosphorProgram.setInt(0, 0);
    
    crtProgram.setInt(1, window_width, window_height);
    crtProgram.setInt(0, 0);
    crtProgram.setInt(3, 8 * max_screen_width, 8 *max_screen_height); // fake out 8-width chars for CRT shader, looks bad otherwise for high-res fonts
    SetTextColor(Config::getEnumOr("TextColor", textColorNames, ETextColor::AMBER));
    
    bloomProgram.setInt(1, window_width, window_height);
    bloomProgram.setInt(0, 0);
    
    LogDebug(firstCompile ? "Shaders Compiled" : "Shaders Recompiled");
    
    {
        
        TextInfo tmp2 = {};
        
        tmp2.screen_width = max_screen_width;
        tmp2.screen_height = max_screen_height;
        
        uint32_t w, h;
        
        auto &fnt = Font::getSelectedFont(w, h, tmp2.char_height, tmp2.char_width, tmp2.font_width, tmp2.font_height);
        
        fontTexture.LoadRGBA8(fnt.data(), w, h);
        
        LogDebug(firstCompile ? "Text Font Loaded" : "Text Font Reloaded");
        
        textBuffer.Init(&tmp2);
        
        textBufferData = reinterpret_cast<TextInfo*>(glMapNamedBufferRange(textBuffer.index, 0, sizeof(TextInfo), GL_MAP_WRITE_BIT | GL_MAP_READ_BIT));
        
        glCheckErrors();
        SetTextColor(Config::getEnumOr("TextColor", textColorNames, ETextColor::AMBER));
        
        glCheckErrorsDebug();
        
        LogDebug(firstCompile ? "Text Info Loaded" : "Text Info Reloaded");
    }
    
    textFrameBuffer.Init(textBufferData->char_width * max_screen_width, textBufferData->char_height * max_screen_height);
    textFrameBuffer2.Init(textBufferData->char_width * max_screen_width, textBufferData->char_height * max_screen_height);
    frameBuffer.Init(window_width, window_height);
    
    mainArea.Gen(std::array { &crtProgram, &bloomProgram } ,-1.0, -1.0, 1.0, 1.0);
    
    textArea.Gen(std::array { &textDrawer, &phosphorProgram } , -1.0, -1.0, 1.0, 1.0);
    
    LogDebug(firstCompile ? "Geometry Generated" : "Geometry Regenerated");
    
    textArea.addUBO(&textBuffer);
    textArea.addTexture(std::array { &fontTexture, &textFrameBuffer.colorTexture });
    mainArea.addTexture(std::array { &textFrameBuffer2.colorTexture, &frameBuffer.colorTexture });
    
    for(size_t i = 0; i < numPhosphorBuffers; i++)
    {
        phosphorBuffers[i].InitNew(textBufferData->char_width * max_screen_width, textBufferData->char_height * max_screen_height);
        textArea.addTexture(std::array { (GLTexture*)nullptr, phosphorBuffers + i });
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
    
    LogDebug("OpenGL version set to 4.6 Core");
    
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

void Renderer::SetText(std::string_view newText)
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


void Renderer::LowRes()
{
    textBufferData->screen_width = 40;
    textBufferData->screen_height = 25;
}

void Renderer::HighRes()
{
    textBufferData->screen_width = 80;
    textBufferData->screen_height = 40;
}

void Renderer::SetText(std::string_view newText, std::span<const uint8_t> properties)
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
    
    uint32_t oldw = textBufferData->char_width;
    uint32_t oldh = textBufferData->char_height;
    
    auto &fnt = Font::getSelectedFont(w, h, textBufferData->char_width, textBufferData->char_height, textBufferData->font_width, textBufferData->font_height);
    
    if(oldw != textBufferData->char_width || oldh != textBufferData->char_height)
    {
        textFrameBuffer.Resize(textBufferData->char_width * max_screen_width, textBufferData->char_height * max_screen_height);
        textFrameBuffer2.Resize(textBufferData->char_width * max_screen_width, textBufferData->char_height * max_screen_height);
        
        //crtProgram.setInt(3, textBufferData->char_width * max_screen_width, textBufferData->char_height * max_screen_height); // fake out 8-width chars for CRT shader, looks bad otherwise for high-res fonts
        
        for(size_t i = 0; i < numPhosphorBuffers; i++)
        {
            phosphorBuffers[i].InitNew(textBufferData->char_width * max_screen_width, textBufferData->char_height * max_screen_height);
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

void Renderer::Render()
{
    uint32_t w = textBufferData->char_width * max_screen_width;
    uint32_t h = textBufferData->char_height * max_screen_height;
    
    textBufferData = nullptr;
    //flush text
    glUnmapNamedBuffer(textBuffer.index);
    
    if(skipNextFrame)
    {
        skipNextFrame = false;
    }
    else
    {
        glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT | GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT);
        
        textDrawer.setUInt(5, Util::MsTime() - baseTime);
        
        phosphorBufferIndex = (phosphorBufferIndex + 1) % numPhosphorBuffers;
        glCopyImageSubData(textFrameBuffer.colorTexture.index, GL_TEXTURE_2D, 0, 0, 0, 0, phosphorBuffers[phosphorBufferIndex].index, GL_TEXTURE_2D, 0, 0, 0, 0, w, h, 1);
        
        for(unsigned i = 0; i < numPhosphorBuffers; i++)
        {
            phosphorProgram.setInt(2 + i,1 + ((phosphorBufferIndex + i) % numPhosphorBuffers));
        }
        
        glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT); // probably not necessary, but...
        
        textArea.Render(std::array {&textFrameBuffer, &textFrameBuffer2});
        mainArea.Render(std::array {&frameBuffer, (GLFrameBuffer*)nullptr});
        
        glCheckErrors();
    }
    
    SDL_GL_SwapWindow(win);
    
    textBufferData = reinterpret_cast<TextInfo*>(glMapNamedBufferRange(textBuffer.index, 0, sizeof(TextInfo), GL_MAP_WRITE_BIT | GL_MAP_READ_BIT));
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
    if(textDrawer.program) textDrawer.setFloat(6, c[0], c[1], c[2], c[3]);
    Config::setEnum("TextColor", textColorNames, color);
}

void Util::ShowFatalError(const std::string &title,const std::string &msg)
{
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title.c_str(), msg.c_str(), window_ok ? win : nullptr);
}


void Renderer::DrawLineText(uint32_t x, uint32_t y, std::string_view newText, uint32_t width)
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

void Renderer::DrawLineTextFillProp(uint32_t x, uint32_t y, std::string_view newText, uint8_t newProperty, uint32_t width)
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

void Renderer::DrawLineProp(uint32_t x, uint32_t y, std::span<const uint8_t> newProperties, uint32_t width)
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

void Renderer::DrawFillLineProp(uint32_t x, uint32_t y, uint8_t newProperty, uint32_t width)
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

void Renderer::DrawFillLineText(uint32_t x, uint32_t y, char newText, uint32_t width)
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

void Renderer::DrawFillLineTextProp(uint32_t x, uint32_t y, char newText, uint8_t newProperty, uint32_t width)
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

void Renderer::DrawChar(uint32_t x, uint32_t y, char newChar, uint8_t newProperty)
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
static void DrawClear(uint32_t x, uint32_t y, uint32_t width)
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

void Renderer::DrawClear(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    for(uint32_t i = 0; i < height; i++)
    {
        ::DrawClear(x, y + i, width);
    }
}

void Renderer::DrawClearText(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    for(uint32_t i = 0; i < height; i++)
    {
        ::DrawClear<true, false>(x, y + i, width);
    }
}

void Renderer::DrawClearProps(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    for(uint32_t i = 0; i < height; i++)
    {
        ::DrawClear<false, true>(x, y + i, width);
    }
}

void Renderer::DrawClear()
{
    size_t bufSiz = max_screen_width * max_screen_height;
    
    memset(textBufferData->chars, 0, bufSiz * 2);
}

void Renderer::DrawClear(uint8_t text_char, uint8_t prop)
{
    size_t bufSiz = max_screen_width * max_screen_height;
    
    memset(textBufferData->chars, text_char, bufSiz);
    memset(textBufferData->char_properties, prop, bufSiz);
}

void Renderer::DrawLineTextCentered(uint32_t y, std::string_view newText, char prop)
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
