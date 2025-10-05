#pragma once

#include "Preprocessor.h"

#include <type_traits>
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdarg>
#include <string>
#include <span>
#include <functional>
#include <variant>
#include <stdexcept>

#include "Log.h"

#ifdef _MSC_VER
    #define ALWAYS_INLINE __forceinline
#elif defined(__GNUC__)
    #define ALWAYS_INLINE inline __attribute__((always_inline))
#else
    #define ALWAYS_INLINE inline
#endif

constexpr uint32_t const_rand(uint32_t &next)
{
    next = next * 1103515245 + 12345;
    return uint32_t(next/65536);
}

template<size_t N>
struct FakeString : std::array<char, N>
{
    constexpr operator std::string_view() const
    {
        return {this->data(), N};
    }
};

inline constexpr char fixChar(char32_t c)
{
    constexpr char32_t straight1 = U'│';
    constexpr char32_t straight2 = U'─';
    constexpr char32_t corner1   = U'┐';
    constexpr char32_t corner2   = U'└';
    constexpr char32_t corner3   = U'┘';
    constexpr char32_t corner4   = U'┌';
    constexpr char32_t inter1    = U'┤';
    constexpr char32_t inter2    = U'┴';
    constexpr char32_t inter3    = U'┬';
    constexpr char32_t inter4    = U'├';
    constexpr char32_t inter5    = U'┼';
    
    constexpr char32_t double_straight1 = U'║';
    constexpr char32_t double_straight2 = U'═';
    constexpr char32_t double_corner1   = U'╗';
    constexpr char32_t double_corner2   = U'╚';
    constexpr char32_t double_corner3   = U'╝';
    constexpr char32_t double_corner4   = U'╔';
    
    constexpr char32_t double_inter1    = U'╣';
    constexpr char32_t double_inter2    = U'╩';
    constexpr char32_t double_inter3    = U'╦';
    constexpr char32_t double_inter4    = U'╠';
    constexpr char32_t double_inter5    = U'╬';
    
    constexpr char32_t block1 = U'░';
    constexpr char32_t block2 = U'▒';
    constexpr char32_t block3 = U'▓';
    constexpr char32_t block4 = U'█';
    
    if(c < 256)
    {
        return c;
    }
    else switch(c)
    {
        //simple
    case straight1:
        return 179;
    case inter1:
        return 180;
    case corner1:
        return 191;
    case corner2:
        return 192;
    case inter2:
        return 193;
    case inter3:
        return 194;
    case inter4:
        return 195;
    case straight2:
        return 196;
    case inter5:
        return 197;
    case corner3:
        return 217;
    case corner4:
        return 218;
        //double
    case double_straight1:
        return 186;
    case double_straight2:
        return 205;
    case double_corner1:
        return 187;
    case double_corner2:
        return 200;
    case double_corner3:
        return 188;
    case double_corner4:
        return 201;
    case double_inter1:
        return 185;
    case double_inter2:
        return 202;
    case double_inter3:
        return 203;
    case double_inter4:
        return 204;
    case double_inter5:
        return 206;
    case block1:
        return 176;
    case block2:
        return 177;
    case block3:
        return 178;
    case block4:
        return 219;
    default:
        return ' ';
    }
}

template<size_t N>
inline consteval FakeString<N - 1> fixString(const char32_t (&str)[N])
{
    FakeString<N - 1> tmp;
    
    //assert(str[N] == 0);
    
    for(size_t i = 0; i < (N - 1); i++)
    {
        tmp[i] = fixChar(str[i]);
    }
    
    return tmp;
}

inline std::string operator ""_fixString(const char32_t * str, size_t len)
{
    std::string tmp;
    tmp.resize(len);
    
    for(size_t i = 0; i < len; i++)
    {
        tmp[i] = fixChar(str[i]);
    }
    
    return tmp;
}


inline std::string fixStringRand(const char32_t * str, char32_t rand_char, uint32_t seed)
{
    size_t len = std::char_traits<char32_t>::length(str);
    uint32_t next = seed;
    std::string tmp;
    tmp.resize(len);
    
    for(size_t i = 0; i < len; i++)
    {
        if(str[i] == rand_char)
        {
            tmp[i] = (const_rand(next) % 253) + 1;
        }
        else
        {
            tmp[i] = fixChar(str[i]);
        }
    }
    
    return tmp;
}

inline std::string stringRand(std::string_view str, char rand_char, uint32_t seed)
{
    uint32_t next = seed;
    std::string tmp;
    tmp.resize(str.size());
    
    for(size_t i = 0; i < str.size(); i++)
    {
        if(str[i] == rand_char)
        {
            tmp[i] = (const_rand(next) % 253) + 1;
        }
        else
        {
            tmp[i] = fixChar(str[i]);
        }
    }
    
    return tmp;
}

inline std::string stringRandDyn(std::string_view str, char rand_char)
{
    std::string tmp;
    tmp.resize(str.size());
    
    for(size_t i = 0; i < str.size(); i++)
    {
        if(str[i] == rand_char)
        {
            tmp[i] = 255;
        }
        else
        {
            tmp[i] = fixChar(str[i]);
        }
    }
    
    return tmp;
}

//chance = 0 - 255, 255 = always
inline std::string stringRandReplace(std::string_view str, char replace, uint32_t chance, uint32_t seed)
{
    uint32_t next = seed;
    std::string tmp;
    tmp.resize(str.size());
    
    for(size_t i = 0; i < str.size(); i++)
    {
        if((const_rand(next) % 256) < chance)
        {
            tmp[i] = replace;
        }
        else
        {
            tmp[i] = fixChar(str[i]);
        }
    }
    
    return tmp;
}

inline constexpr std::string operator ""_s(const char * s,size_t n){
    return {s,n};
}

inline constexpr std::string_view operator ""_sv(const char * s,size_t n){
    return {s,n};
}

inline consteval unsigned long long int operator ""_K(unsigned long long int n){
    return n*1024;
}

inline consteval unsigned long long int operator ""_M(unsigned long long int n){
    return n*1024_K;
}

inline consteval unsigned long long int operator ""_G(unsigned long long int n){
    return n*1024_M;
}

struct TracedError : std::runtime_error
{
    std::string trace;
    
    TracedError(const std::string &what) : runtime_error(what), trace(CaptureTrace()) {}
    TracedError(const char *what) : runtime_error(what), trace(CaptureTrace()) {}
    
    TracedError(const std::string &what, int skip) : runtime_error(what), trace(CaptureTrace(skip)) {}
    TracedError(const char *what, int skip) : runtime_error(what), trace(CaptureTrace(skip)) {}
    
    static std::string CaptureTrace(int skip = 0);
};

struct FatalError : TracedError
{
    using TracedError::TracedError;
};

struct RecoverableError : TracedError
{
    using TracedError::TracedError;
};

namespace Util
{
    template<typename T, typename V>
    concept ConvertibleTo = std::convertible_to<std::remove_cvref_t<T>, V>;
    
    template<typename T, typename V>
    concept ContainerOf = std::same_as<std::remove_cvref_t<decltype(*std::begin(std::declval<T>()))>, V>;
    
    template<typename T, typename V>
    concept ContainerConvertibleTo = std::convertible_to<std::remove_cvref_t<decltype(*std::begin(std::declval<T>()))>, V>;
    
    template<typename T, typename V>
    concept ContainerComparableTo = std::equality_comparable_with<std::remove_cvref_t<decltype(*std::begin(std::declval<T>()))>, V>;
    
    template<auto Init, auto Quit>
    struct Guard
    {
        const char * name;
        Guard(const char * name) : name(name)
        {
            LogVerbose("Starting Guard for %s", name);
            Init();
        }
        
        ~Guard()
        {
            LogVerbose("Stopping Guard for %s", name);
            Quit();
        }
    };
    
    
    template<std::ranges::input_range Ra, ContainerConvertibleTo<typename Ra::value_type> ... Rb>
    std::vector<typename Ra::value_type> Concat(const Ra &ra,const Rb & ... rb)
    {
        std::vector<typename Ra::value_type> o(std::begin(ra), std::end(ra));
        o.reserve(std::size(ra)+(std::size(rb)+...));
        (std::copy(std::begin(rb),std::end(rb),std::back_inserter(o)),...);
        return o;
    }
    
    template<std::ranges::input_range Ra, ContainerConvertibleTo<typename Ra::value_type> ... Rb>
    auto ConcatInplace(Ra &&ra,const Rb & ... rb)
    {
        ra.reserve(std::size(ra)+(std::size(rb)+...));
        (std::copy(std::begin(rb),std::end(rb),std::back_inserter(ra)),...);
        return std::forward<Ra&&>(ra);
    }
    
    template<ContainerConvertibleTo<std::string> T>
    std::string JoinOr(T v,std::string_view sep_comma=", ", std::string_view sep_or=", or ")
    {
        std::string o;
        const size_t n = v.size();
        
        if(n == 1) return v[0];
        
        for(size_t i = 0; i < n; i++)
        {
            if(i == (n - 1))
            {
                o += sep_or;
            }
            else if(i > 0)
            {
                o += sep_comma;
            }
            o += v[i];
        }
        return o;
    }
    
    template<ContainerConvertibleTo<std::string> T>
    std::string Join(T v,std::string_view sep=", ")
    {
        std::string o;
        const size_t n = v.size();
        
        if(n == 1) return v[0];
        
        for(size_t i = 0; i < n; i++)
        {
            if(i > 0)
            {
                o += sep;
            }
            o += v[i];
        }
        return o;
    }
    
    template<typename T>
    auto MaxAll(T a) -> std::remove_cvref_t<decltype(a[0])>
    {
        size_t n = std::size(a);
        
        if(n == 0) return 0;
        
        std::remove_cvref_t<decltype(a[0])> m = a[0];
        
        for(size_t i = 1; i < n; i++)
        {
            m = std::max(m, a[i]);
        }
        
        return m;
    }
    
    template<typename R, typename Fn>
    ALWAYS_INLINE auto invoke_all(R &&val, const Fn &fn)
    {
        return std::invoke(fn, std::forward<R>(val));
    }
    
    template<typename R, typename Fn1, typename Fn2, typename... Fns>
    ALWAYS_INLINE auto invoke_all(R &&val, Fn1&& fn1, Fn2&& fn2, Fns&&... fns)
    {
        return invoke_all<R, Fn2, Fns...>(std::invoke(fn1, std::forward<R>(val)));
    }
    
    template<typename R,typename... Fns>
    auto Filter(R &&r,Fns&&... fns)
    {
        std::vector<typename R::value_type> v;
        v.reserve(std::size(r));
        for(auto &e:r)
        {
            if((std::invoke(fns, e) && ...))
            {
                if constexpr(std::is_rvalue_reference_v<decltype(r)>)
                {
                    v.emplace_back(std::move(e));
                }
                else
                {
                    v.push_back(e);
                }
            }
        }
        return v;
    }
    
    template<typename R,typename... Fns>
    auto Map(R &&r,Fns&&... fns)
    {
        std::vector<decltype(invoke_all(r[0], fns...))> v;
        v.reserve(std::size(r));
        for(auto &e:r)
        {
            v.emplace_back(invoke_all(e, fns...));
        }
        return v;
    }
    
    template<typename R,typename... Fns>
    auto MapInplace(R &&r, Fns&&... fns)
    {
        for(auto &e:r)
        {
            e = invoke_all(e, fns...);
        }
        return std::forward<R&&>(r);
    }
    
    template<typename T, typename R, typename Fn>
    auto Reduce(R &&r, Fn&& fn)
    {
        T v = {};
        
        for(auto &e:r)
        {
            v = fn(e, v);
        }
        
        return v;
    }
    
    template<typename R, typename Fn>
    auto SortInplace(R &&r, Fn&& fn)
    {
        std::sort(std::begin(r), std::end(r), fn);
        return std::forward<R&&>(r);
    }
    
    template<typename R,typename... Fns>
    void ForEach(R &&r, Fns&&... fns)
    {
        for(auto &e:r)
        {
            invoke_all(e, fns...);
        }
    }
    
    class VAList_Guard
    {
        std::va_list &list;
    public:
        
        VAList_Guard(std::va_list &l) : list(l) {}
        
        ~VAList_Guard()
        {
            va_end(list);
        }
    };
    
    #define VAStart(name, argN) std::va_list name; va_start(name, argN); Util::VAList_Guard PP_JOIN(guard,__LINE__)(name)
    #define VACopy(list, name) std::va_list name; va_copy(name, list); Util::VAList_Guard PP_JOIN(guard,__LINE__)(name)
    
    inline std::string VFormat(const char * fmt, std::va_list args)
    {
        VACopy(args, args2);
        size_t len = vsnprintf(nullptr, 0, fmt, args2);
        std::string temp;
        temp.resize(len);
        vsnprintf(temp.data(), len + 1, fmt, args);
        return temp;
    }
    
    inline std::string Format(const char * fmt, ...) __attribute__((format(printf,1,2)));
    
    inline std::string Format(const char * fmt, ...)
    {
        VAStart(args, fmt);
        return VFormat(fmt, args);
    }
    
    std::string QuoteString(std::string_view str, char quote_char = '\'', bool escape_slash = true);
    
    std::string ReadFile(const std::string &filename);
    std::vector<std::byte> ReadFileBinary(const std::string &filename);
    void WriteFile(const std::string &filename, std::string_view data);
    void WriteFileBinary(const std::string &filename, std::span<const std::byte> data);
    
    void ShowFatalError(const std::string &title,const std::string &msg);
    
    //returns rgba8 array
    std::vector<uint32_t> ReadFileBitmap(const std::string &filename, uint32_t &width, uint32_t &height);
    
    uint64_t MsTime();
    
    std::vector<std::string_view> SplitLines(std::string_view text, uint32_t maxWidth);
    
    struct SplitPoint
    {
        size_t offset;
        size_t orig_len;
        std::string str;
        
        std::string_view to_view() const
        {
            return str;
        }
    };
    
    struct SplitOp
    {
        size_t offset;
        std::string op;
    };
    
    struct SplitQuote
    {
        char c; // 0 if not quoted, '\'' if single quote, '"' if double quote
        std::string str;
        std::vector<bool> was_escaped; // if any particular char was escaped
    };
    
    std::vector<SplitPoint> SplitStringEx(std::string_view str, char split_on = ' ', bool join_empty = false, bool use_quotes = true, bool keep_quotes = false, const std::vector<std::pair<char,char>> &braces = {});
    
    std::vector<std::string> SplitString(std::string_view str, char split_on = ' ', bool join_empty = false, bool use_quotes = true, bool keep_quotes = false, const std::vector<std::pair<char,char>> &braces = {});
    
    std::vector<std::string> SplitString(std::string_view str, const std::string &split_on, bool join_empty = false, bool use_quotes = true, bool keep_quotes = false, const std::vector<std::pair<char,char>> &braces = {});
    
    std::vector<std::variant<SplitOp, SplitPoint>> SplitStringOp(std::string_view str, const std::vector<std::string> &ops);
    
    std::vector<SplitQuote> SplitStringQuotes(std::string_view str);
    
    template<typename T>
    auto MapValues(T &&t)
    {
        std::vector<typename std::remove_cvref_t<T>::mapped_type> vals;
        for(auto &entry:t)
        {
            vals.push_back(entry.second);
        }
        return vals;
    }
    
    char CharToLower(char c);
    char CharToUpper(char c);
    
    std::string StrToLower(std::string_view str);
    std::string StrToUpper(std::string_view str);
    
    inline std::string ReplaceChars(std::string s, char from, char to)
    {
        std::replace(s.begin(), s.end(), from, to);
        return s;
    }
    
    
    std::string Decompress(std::span<const std::byte> data);
    std::vector<std::byte> Compress(std::string_view str);
}

#define UseSubsystem(x) Util::Guard<x::Init, x::Quit> PP_JOIN(manager_,__LINE__) {#x}
