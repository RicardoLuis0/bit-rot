#include "Common.h"
#include "Bitmap.h"

#include <fstream>
#include <sstream>
#include <cstring>
#include <iterator>

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
        if(!f) throw std::runtime_error(strerror(errno));
        ss << f.rdbuf();
        return ss.str();
    }
    catch(std::exception &e)
    {
        throw std::runtime_error("Failed to open/read file "+Util::QuoteString(filename)+" : "+e.what());
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
        throw std::runtime_error("Failed to open/read file "+Util::QuoteString(filename)+" : "+e.what());
    }
    
    void WriteFile(const std::string & filename, std::string_view data) try
    {
        std::ofstream f(filename);
        f << data;
    }
    catch(std::exception &e)
    {
        throw std::runtime_error("Failed to open/write file "+Util::QuoteString(filename)+" : "+e.what());
    }
    
    void WriteFileBinary(const std::string & filename, std::span<std::byte> data) try
    {
        std::ofstream f(filename, std::ios::binary);
        f.write(reinterpret_cast<const char*>(data.data()), data.size());
    }
    catch(std::exception &e)
    {
        throw std::runtime_error("Failed to open/write file "+Util::QuoteString(filename)+" : "+e.what());
    }
    
    uint32_t MsTime()
    {
        //return SDL_GetTicks();
        return uint32_t((double(clock()) /  CLOCKS_PER_SEC) * 1000.0); // read clock instead of ticks, so that it isn't affected by speedhacks/etc
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
    void SplitStringInternal(const std::string &str, bool join_empty, bool use_quotes, FnCheck &&isSpace, FnFound &&found)
    {
        size_t i = 0;
        size_t lastSection = 0;
        size_t lastStart = 0;
        char lastQuoteType = 0;
        
        std::string tmp;
        bool has_tmp = false;
        
        for(i = 0; i < std::size(str); i++)
        {
            if(isSpace(str[i]))
            {
                found(lastStart, i - lastStart, tmp, std::string_view(str).substr(lastSection, i - lastSection));
                lastSection = i + 1;
                tmp = "";
                has_tmp = false;
                if(join_empty) for(;(i + 1) < std::size(str) && isSpace(str[i + 1]); i++, lastSection++);
                lastStart = i + 1;
            }
            else if(use_quotes && (str[i] == '\'' || str[i] == '"'))
            {
                lastQuoteType = str[i];
                tmp += str.substr(lastSection, i - lastSection);
                has_tmp = true;
                bool wasQuote = false;
                
                for(i++;(str[i] != lastQuoteType || wasQuote) && i < std::size(str); i++)
                {
                    if(i == '\\' && !wasQuote)
                    {
                        wasQuote = true;
                    }
                    else
                    {
                        tmp += wasQuote ? unescape(i) : str[i];
                        wasQuote = false;
                    }
                }
                lastSection = i + 1;
            }
        }
        
        if(lastSection != i)
        { // ended on an unquoted section
            found(lastStart, i - lastStart, tmp, std::string_view(str).substr(lastSection));
        }
        else if(has_tmp)
        { // ended on a quote
            found(lastStart, i - lastStart, tmp, {});
        }
    }
    
    std::vector<SplitPoint> SplitStringEx(const std::string &str, char split_on, bool join_empty, bool use_quotes)
    {
        std::vector<SplitPoint> o;
        
        SplitStringInternal(str, join_empty, use_quotes,
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
    
    std::vector<std::string> SplitString(const std::string &str, char split_on, bool join_empty, bool use_quotes)
    {
        std::vector<std::string> o;
        
        SplitStringInternal(str, join_empty, use_quotes,
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
    
    std::vector<std::string> SplitString(const std::string &str, const std::string &split_on, bool join_empty, bool use_quotes)
    {
        std::vector<std::string> o;
        
        SplitStringInternal(str, join_empty, use_quotes,
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
    
    char clower(char c)
    {
        return c >= 'A' && c <= 'Z' ? c - ('A' - 'a') : c;
    }
    
    char cupper(char c)
    {
        return c >= 'a' && c <= 'z' ? c + ('A' - 'a') : c;
    }
    
    std::string StrToLower(const std::string &str)
    {
        std::string s = str;
        MapInplace(s, clower);
        return s;
    }
    
    std::string StrToUpper(const std::string &str)
    {
        std::string s = str;
        MapInplace(s, cupper);
        return s;
    }
}
