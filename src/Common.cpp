#include "Common.h"
#include "Bitmap.h"

#include <cinttypes>
#include <bit>
#include <fstream>
#include <sstream>
#include <cstring>
#include <iterator>
#include <chrono>

#include <zlib.h>

#include <cxxabi.h>
#include <backtrace.h>

backtrace_state * trace_state = nullptr;

struct trace_info
{
    std::string str;
    int num = 0;
};

void trace_create_state_callback(void * data, const char * msg, int errnum)
{
    std::string * str = reinterpret_cast<std::string*>(data);
    
    (*str) += (msg ? std::string(msg) + " (" : "(") + std::to_string(errnum) + ")";
}

void trace_callback_err(void * data, const char * msg, int errnum)
{
    trace_info * info = reinterpret_cast<trace_info*>(data);
    
    info->str += (msg ? std::string(msg) + " (" : "(") + std::to_string(errnum) + ")";
}

char char_buffer[4096];

std::string strip_filename(std::string fname)
{
    size_t src1 = fname.rfind("/src/");
    size_t src2 = fname.rfind("\\src\\");
    size_t inc1 = fname.rfind("/include/");
    size_t inc2 = fname.rfind("\\include\\");
    
    if(src1 == std::string::npos) src1 = 0;
    if(src2 == std::string::npos) src2 = 0;
    if(inc1 == std::string::npos) inc1 = 0;
    if(inc2 == std::string::npos) inc2 = 0;
    
    return fname.substr(std::max({src1, src2, inc1, inc2}));
}

char * hex_to_str(uint64_t hexval)
{
    snprintf(char_buffer, 1024, "0x%" PRIx64, hexval);
    return char_buffer;
}

int trace_callback(void *data, uintptr_t pc, const char *filename, int lineno, const char *function)
{
    trace_info * info = reinterpret_cast<trace_info*>(data);
    
    int ok = 0;
    if(function)
    {
        char * demangled = abi::__cxa_demangle(function, nullptr, nullptr, &ok);
        
        std::string fn_name(ok == 0 ? demangled : function);
        if(filename)
        {
            info->str += "#" + std::to_string(info->num) + " [" + fn_name + " @ " + strip_filename(filename) + ":" + std::to_string(lineno) + "] " + hex_to_str(pc) + "\n";
        }
        else
        {
            info->str += "#" + std::to_string(info->num) + " [" + fn_name + " @ ??? :" + std::to_string(lineno) + "] 0x" + hex_to_str(pc)  + "\n";
        }
        
        if(demangled) free(demangled);
    }
    else if(filename)
    {
        info->str += "#" + std::to_string(info->num) + " [ ??? @ " + strip_filename(filename) + ":" + std::to_string(lineno) + "] 0x" + hex_to_str(pc)  + "\n";
    }
    else
    {
        info->str += "#" + std::to_string(info->num) + " [ ??? ] 0x" + hex_to_str(pc)  + "\n";
    }
    info->num++;
    
    return 0;
}

std::string FatalError::CaptureTrace(int skip)
{
    if(!trace_state)
    {
        std::string state_err;
        trace_state = backtrace_create_state(nullptr, 0, trace_create_state_callback, reinterpret_cast<void*>(&state_err));
        
        if(!trace_state)
        {
            return state_err;
        }
    }
    trace_info info;
    backtrace_full(trace_state, skip + 2, trace_callback, trace_callback_err, reinterpret_cast<void*>(&info));
    return info.str;
}

namespace Util
{
    
    static constexpr char escape(char c)
    {
        switch(c)
        {
        case '\a':
            return 'a';
        case '\b':
            return 'b';
        case '\e':
            return 'e';
        case '\f':
            return 'f';
        case '\n':
            return 'n';
        case '\r':
            return 'r';
        case '\t':
            return 't';
        case '\v':
            return 'v';
        default:
            return c;
        }
    }
    static constexpr char needs_escape(char c)
    {
        switch(c)
        {
        case '\a':
        case '\b':
        case '\e':
        case '\f':
        case '\n':
        case '\r':
        case '\t':
        case '\v':
        case '\\':
            return true;
        default:
            return false;
        }
    }
    
    std::string QuoteString(std::string_view s, char quote_char, bool escape_slash)
    {
        std::string str;
        str.reserve(s.size() * 1.5);
        
        str += quote_char;
        
        for(char c : s)
        {
            if(c == quote_char || (needs_escape(c) && (escape_slash || c!='\\')))
            {
                str += '\\';
                str += escape(c);
            }
            else
            {
                str += c;
            }
        }
        str += quote_char;
        return str;
    }
    
    std::string ReadFile(const std::string &filename) try
    {
        std::ostringstream ss;
        std::ifstream f(filename);
        if(!f) throw FatalError(strerror(errno));
        ss << f.rdbuf();
        return ss.str();
    }
    catch(std::exception &e)
    {
        throw FatalError("Failed to open/read file "+Util::QuoteString(filename)+" : "+e.what());
    }
    
    std::vector<std::byte> ReadFileBinary(const std::string &filename) try
    {
        std::ifstream f(filename, std::ios::binary | std::ios::ate);
        size_t len = f.tellg();
        f.seekg(0);
        std::vector<std::byte> data;
        data.resize(len);
        f.read(reinterpret_cast<char*>(data.data()), len);
        return data;
    }
    catch(std::exception &e)
    {
        throw FatalError("Failed to open/read file "+Util::QuoteString(filename)+" : "+e.what());
    }
    
    void WriteFile(const std::string & filename, std::string_view data) try
    {
        std::ofstream f(filename);
        f << data;
    }
    catch(std::exception &e)
    {
        throw FatalError("Failed to open/write file "+Util::QuoteString(filename)+" : "+e.what());
    }
    
    void WriteFileBinary(const std::string & filename, std::span<const std::byte> data) try
    {
        std::ofstream f(filename, std::ios::binary);
        f.write(reinterpret_cast<const char*>(data.data()), data.size());
    }
    catch(std::exception &e)
    {
        throw FatalError("Failed to open/write file "+Util::QuoteString(filename)+" : "+e.what());
    }
    
    std::vector<std::string_view> SplitLines(std::string_view text, uint32_t maxWidth)
    {
        std::vector<std::string_view> lines;
        size_t lastSplit = 0;
        size_t lastSpace = 0;
        size_t i = 0;
        for(i = 0; i < std::size(text); i++)
        {
            if(text[i] == ' ') lastSpace = i;
            
            if(text[i] == '\n')
            {
                lines.push_back(text.substr(lastSplit, i - lastSplit));
                lastSplit = i + 1;
            }
            else if((i - lastSplit) >= maxWidth)
            {
                lines.push_back(text.substr(lastSplit, lastSpace - lastSplit));
                lastSplit = lastSpace + 1;
            }
        }
        if(i != lastSplit)
        {
            lines.push_back(text.substr(lastSplit));
        }
        return lines;
    }
    
    constexpr char unescape(char c)
    {
        switch(c)
        {
        case 'a':
            return '\a';
        case 'b':
            return '\b';
        case 'e':
            return '\e';
        case 'f':
            return '\f';
        case 'n':
            return '\n';
        case 'r':
            return '\r';
        case 't':
            return '\t';
        case 'v':
            return '\v';
        case '\\':
            return '\\';
        case '"':
            return '\"';
        default:
            return c;
        }
    }
    
    
    template<typename FnCheck, typename FnFound>
    void SplitStringInternal(std::string_view str, bool join_empty, bool use_quotes, bool keep_quotes, FnCheck &&isSpace, FnFound &&found)
    {
        size_t i = 0;
        size_t lastSection = 0;
        size_t lastStart = 0;
        char lastQuoteType = 0;
        
        std::string tmp;
        bool has_tmp = false;
        
        if(join_empty && std::size(str) > 0 && isSpace(str[i])) for(; i < std::size(str) && isSpace(str[i]); i++, lastSection++);
        
        for(; i < std::size(str); i++)
        {
            if(isSpace(str[i]))
            {
                found(lastStart, i - lastStart, tmp, str.substr(lastSection, i - lastSection));
                lastSection = i + 1;
                tmp = "";
                has_tmp = false;
                if(join_empty) for(;(i + 1) < std::size(str) && isSpace(str[i + 1]); i++, lastSection++);
                lastStart = i + 1;
            }
            else if(use_quotes && (str[i] == '\'' || str[i] == '"'))
            {
                bool wasSlash = false;
                
                if(keep_quotes)
                {
                    for(i++;(str[i] != lastQuoteType || wasSlash) && i < std::size(str); i++)
                    {
                        if(i == '\\' && !wasSlash)
                        {
                            wasSlash = true;
                        }
                        else
                        {
                            wasSlash = false;
                        }
                    }
                }
                else
                {
                    lastQuoteType = str[i];
                    tmp += str.substr(lastSection, i - lastSection);
                    has_tmp = true;
                    
                    for(i++;(str[i] != lastQuoteType || wasSlash) && i < std::size(str); i++)
                    {
                        if(i == '\\' && !wasSlash)
                        {
                            wasSlash = true;
                        }
                        else
                        {
                            tmp += wasSlash ? unescape(str[i]) : str[i];
                            wasSlash = false;
                        }
                    }
                    lastSection = i + 1;
                }
            }
        }
        
        if(lastSection != i)
        { // ended on an unquoted section
            found(lastStart, i - lastStart, tmp, str.substr(lastSection));
        }
        else if(has_tmp)
        { // ended on a quote
            found(lastStart, i - lastStart, tmp, {});
        }
    }
    
    std::vector<SplitPoint> SplitStringEx(std::string_view str, char split_on, bool join_empty, bool use_quotes, bool keep_quotes)
    {
        std::vector<SplitPoint> o;
        
        SplitStringInternal(str, join_empty, use_quotes, keep_quotes,
        [split_on](char c)
        {
            return c == split_on;
        },
        [&o](size_t start, size_t len, const std::string &tmp, std::string_view str)
        {
            SplitPoint p {.offset = start, .orig_len = len, .str = std::string_view {}};
            if(tmp.empty())
            {
                p.str = str;
            }
            else
            {
                p.str = (tmp + std::string(str));
            }
            o.emplace_back(std::move(p));
        });
        
        return o;
    }
    
    std::vector<std::string> SplitString(std::string_view str, char split_on, bool join_empty, bool use_quotes, bool keep_quotes)
    {
        std::vector<std::string> o;
        
        SplitStringInternal(str, join_empty, use_quotes, keep_quotes,
        [split_on](char c)
        {
            return c == split_on;
        },
        [&o](size_t start, size_t len, const std::string &tmp, std::string_view str)
        {
            o.push_back(tmp + std::string(str));
        });
        
        return o;
    }
    
    std::vector<std::string> SplitString(std::string_view str, const std::string &split_on, bool join_empty, bool use_quotes, bool keep_quotes)
    {
        std::vector<std::string> o;
        
        SplitStringInternal(str, join_empty, use_quotes, keep_quotes,
        [&split_on](char c)
        {
            return split_on.contains(c);
        },
        [&o](size_t start, size_t len, const std::string &tmp, std::string_view str)
        {
            o.push_back(tmp + std::string(str));
        });
        
        return o;
    }
    
    char CharToLower(char c)
    {
        return c >= 'A' && c <= 'Z' ? c - ('A' - 'a') : c;
    }
    
    char CharToUpper(char c)
    {
        return c >= 'a' && c <= 'z' ? c + ('A' - 'a') : c;
    }
    
    std::string StrToLower(std::string_view str)
    {
        std::string s(str);
        MapInplace(s, CharToLower);
        return s;
    }
    
    std::string StrToUpper(std::string_view str)
    {
        std::string s(str);
        MapInplace(s, CharToUpper);
        return s;
    }
    std::string Decompress(std::span<const std::byte> data)
    {
        std::string out;
        
        constexpr const int bufSiz = 256;
        char buf[bufSiz];
        
        z_stream z;
        z.zalloc = nullptr;
        z.zfree = nullptr;
        z.opaque = nullptr;
        z.avail_in = 0;
        z.next_in = nullptr;
        if(int err = inflateInit(&z); err != Z_OK)
        {
            throw FatalError("ZLib Decompress Failed: "+std::to_string(err));
        }
        
        z.avail_in = data.size();
        z.next_in = (unsigned char *)(data.data());
        
        do
        {
            z.avail_out = bufSiz;
            z.next_out = reinterpret_cast<unsigned char *>(buf);
            int err = inflate(&z, Z_NO_FLUSH);
            if(err != Z_OK && err != Z_STREAM_END)
            {
                throw FatalError("ZLib Decompress Failed: "+std::to_string(err));
            }
            int bufFill = bufSiz - z.avail_out;
            out += std::string_view(buf, bufFill);
            if(err == Z_STREAM_END) break;
        }
        while(z.avail_out == 0);
        
        return out;
    }
    
    std::vector<std::byte> Compress(std::string_view str)
    {
        auto maxLen = compressBound(str.length());
        std::vector<std::byte> buf;
        buf.resize(maxLen);
        unsigned long realLen = maxLen;
        if(int err = compress2(reinterpret_cast<unsigned char *>(buf.data()), &realLen, (unsigned char *)(str.data()), str.length(), Z_BEST_COMPRESSION); err != Z_OK)
        {
            throw FatalError("ZLib Compress Failed: "+std::to_string(err));
        }
        buf.resize(realLen);
        return buf;
    }
}

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#endif

namespace Util
{
    bool hasFreq = false;
    uint64_t PerformanceFreq;
    uint64_t MsTime()
    {
        #ifdef _WIN32
            LARGE_INTEGER i;
            if(hasFreq)
            {
                QueryPerformanceCounter(&i);
                return std::bit_cast<uint64_t>(i.QuadPart) / PerformanceFreq;
            }
            else
            {
                QueryPerformanceFrequency(&i);
                PerformanceFreq = std::bit_cast<uint64_t>(i.QuadPart) / 1000;
                hasFreq = true;
                return MsTime();
            }
        #else
            return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
        #endif
    }
    
    extern "C" uint64_t Custom_GetTics()
    {
        return MsTime();
    }
}
