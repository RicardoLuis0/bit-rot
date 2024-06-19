/**
  * Permission is hereby granted, free of charge, to any person obtaining a copy of this
  * software and associated documentation files (the "Software"), to deal in the Software
  * without restriction, including without limitation the rights to use, copy, modify,
  * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
  * permit persons to whom the Software is furnished to do so.
  *
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
  * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
  * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
  * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
  * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
  * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
  */

#include "json.h"
#include <bit>
#include <cmath>
#include <cstring>
#include <stdexcept>

namespace JSON
{
    namespace
    {
        constexpr bool is_whitespace(char c)
        {
            return c == ' ' || c == '\t' || c == '\r' ||c == '\n';
        }
        
        constexpr bool is_word_start(char c)
        {
            return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
        }
        
        constexpr bool is_number(char c)
        {
            return (c >= '0' && c <= '9');
        }
        
        constexpr bool is_word_char(char c)
        {
            return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c=='_';
        }
        
        bool is_number_start_nosign(std::string_view data, size_t i)
        {
            return is_number(data[i])
                    || (data[i] == '.' && (i + 1) < data.size()
                            &&is_number(data[i + 1]));
        }
        
        bool is_number_start(std::string_view data, size_t i)
        {
            return is_number_start_nosign(data, i)
                    || ((data[i] == '-' || data[i] == '+') && (i + 1) < data.size()
                            && is_number_start_nosign(data, i + 1));
        }
        
        Element get_number(std::string_view data, size_t &i)
        {   //handles integers, decimals and scientific notation
            
            //the way this parses floating point is cursed, and **WILL** destroy precision
            //TODO: replace it with something sensible
            //(i wrote this many years ago, before i knew better T.T)
            
            if(i >= data.size()) throw std::runtime_error("Expected Number, got EOF");
            
            union
            {
                uint64_t i = 0;
                double d;
            } num;
            
            bool is_double = false;
            bool valid = false;
            bool is_negative = data[i] == '-';
            ssize_t double_depth = 1;
            
            if(data[i] == '-' || data[i] == '+') i++;
            
            for(;i < data.size(); i++)
            {
                if(!is_double && data[i] == '.')
                {
                    is_double = true;
                    num.d = num.i;
                }
                else if(data[i] == 'e' || data[i] == 'E')
                {
                    i++;
                    bool negative = false;
                    ssize_t exponent = 0;
                    valid = false;
                    if(i < data.size() && (data[i] == '+' || data[i] == '-'))
                    {
                        if(data[i] == '-') negative = true;
                        i++;
                    }
                    for(;i < data.size(); i++)
                    {
                        if(data[i] >= '0' && data[i] <= '9')
                        {
                            valid = true;
                            (exponent *= 10) += data[i] - '0';
                        }
                        else
                        {
                            break;
                        }
                    }
                    if(is_double)
                    {
                        num.d = num.d * std::pow(10, negative ? -exponent : exponent);
                    }
                    else
                    {
                        num.d = num.i * std::pow(10, negative ? -exponent : exponent);
                        is_double = true;
                    }
                    break;
                }
                else if(data[i] >= '0' && data[i] <= '9')
                {
                    valid = true;
                    if(is_double)
                    {
                        num.d += (data[i] - '0') * std::pow(10, -(double_depth++));
                    }
                    else
                    {
                        (num.i *= 10) += data[i] - '0';
                    }
                }
                else
                {
                    break;
                }
            }
            
            if(!valid)
            {
                if(data.size() >= i)
                {
                    throw std::runtime_error("Expected Number, got EOF");
                }
                else
                {
                    throw std::runtime_error("Expected Number, got '"_s + data[i] + "' at pos " + std::to_string(i));
                }
            }
            else if(is_double)
            {
                return is_negative ? -num.d : num.d;
            }
            else
            {
                return is_negative ? -std::bit_cast<int64_t>(num.i) : std::bit_cast<int64_t>(num.i);
            }
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
        
        std::string QuoteChar(char c)
        {
            switch(c)
            {
            case '\a':
                return "'\\a'";
            case '\b':
                return "'\\b'";
            case '\e':
                return "'\\e'";
            case '\f':
                return "'\\f'";
            case '\n':
                return "'\\n'";
            case '\r':
                return "'\\r'";
            case '\t':
                return "'\\t'";
            case '\v':
                return "'\\v'";
            case '\\':
                return "single quote";
            case '"':
                return "double quote";
            default:
                return "'"_s + c + "'";
            }
        }
        
        inline bool is_char(std::string_view data, size_t &i, char c)
        {
            return (i < data.size()) && (data[i] == c);
        }
        
        inline void expect_char(std::string_view data, size_t &i, char c)
        {
            if(!is_char(data, i, c))  throw std::runtime_error("Expected " + QuoteChar(c) + ", got " + (i >= data.size() ? "EOF" : QuoteChar(data[i])+" at pos "+std::to_string(i)));
            i++;
        }
        
        std::string get_string(std::string_view data, size_t &i)
        {   //TODO: optimize string reading
            expect_char(data,i,'"');
            
            size_t start = i;
            size_t n = 0;
            
            for(;i < data.size(); i++)
            {
                if(data[i] == '\n')
                {
                    continue;
                }
                else if(data[i] == '\\')
                {
                    i++;
                    n++;
                }
                else if(data[i] == '"')
                {
                    std::string str;
                    str.reserve(n);
                    for(size_t j = start; j < i; j++)
                    {
                        if(data[j] == '\n')
                        {
                            continue;
                        }
                        else if(data[j] == '\\')
                        {
                            j++;
                            str += unescape(data[j]);
                        }
                        else
                        {
                            str += data[j];
                        }
                    }
                    i++;
                    return str;
                }
                else
                {
                    n++;
                }
            }
            throw std::runtime_error("Expected '\"', got EOF");
        }
        
        void skip_whitespace(std::string_view data, size_t &i)
        {   //SAFE TO CALL ON EOF
            while(i < data.size())
            {
                if(is_whitespace(data[i]))
                {
                    i++;
                }
                else if(data[i] == '#')
                {
                    i++;
                    for(;i < data.size() && data[i - 1] != '\n'; i++);
                }
                else if(data[i] == '/' && (i + 1) < data.size() && (data[i + 1] == '/' || data[i + 1] == '*'))
                {
                    i++;
                    if(data[i] == '/')
                    {
                        i += 2;
                        for(;i < data.size() && data[i - 1] != '\n'; i++);
                    }
                    else
                    {
                        i += 3;
                        for(;i < data.size() && !(data[i - 2] == '*' && data[i - 1] == '/'); i++);
                    }
                }
                else
                {
                    break;
                }
            }
        }
        
        Element get_element(std::string_view data, size_t &i);
        
        array_t get_array(std::string_view data, size_t &i)
        {
            expect_char(data, i, '[');
            
            skip_whitespace(data, i);
            
            if(is_char(data, i, ']'))
            {
                i++;
                return {};
            }
            
            std::vector<Element> v;
            
            while(i<data.size())
            {
                skip_whitespace(data, i);
                
                v.emplace_back(get_element(data, i));
                
                skip_whitespace(data, i);
                
                if(i < data.size() && data[i] == ']')
                {
                    i++;
                    return v;
                }
                
                expect_char(data, i, ',');
                
                skip_whitespace(data, i);
                
                if(i < data.size() && data[i] == ']')
                {   // trailing comma
                    i++;
                    return v;
                }
            }
            throw std::runtime_error("Expected ']', got EOF");
        }
        
        object_t get_object(std::string_view data, size_t &i)
        {
            expect_char(data, i, '{');
            
            skip_whitespace(data, i);
            
            if(is_char(data, i, '}'))
            {
                i++;
                return {};
            }
            
            std::map<std::string,Element> m;
            
            while(i<data.size())
            {
                skip_whitespace(data, i);
                
                std::string key = get_string(data,i);
                
                skip_whitespace(data, i);
                
                expect_char(data, i, ':');
                
                skip_whitespace(data, i);
                
                m.insert({key, get_element(data, i)});
                
                skip_whitespace(data, i);
                
                if(i < data.size() && data[i] == '}')
                {
                    i++;
                    return m;
                }
                
                expect_char(data, i, ',');
                
                skip_whitespace(data, i);
                
                if(i < data.size() && data[i] == '}')
                {   // trailing comma
                    i++;
                    return m;
                }
            }
            throw std::runtime_error("Expected '}', got EOF");
        }
        
        Element get_element(std::string_view data, size_t &i)
        {
            skip_whitespace(data, i);
            if(i >= data.size()) throw std::runtime_error("Expected JSON, got EOF");
            switch(data[i])
            {
            case '[':
                return JSON::Array(get_array(data, i));
            case '{':
                return JSON::Object(get_object(data, i));
            case '"':
                return get_string(data, i);
            default:
                if(is_number_start(data,i))
                {
                    return get_number(data, i);
                }
                else if( data.size() > (i + 3)
                        && data[i]      == 'n'
                        && data[i + 1]  == 'u'
                        && data[i + 2]  == 'l'
                        && data[i + 3]  == 'l'
                ) {
                    i+=4;
                    return JSON_NULL;
                }
                else if(data.size() > (i + 3)
                        && data[i]      == 't'
                        && data[i + 1]  == 'r'
                        && data[i + 2]  == 'u'
                        && data[i + 3]  == 'e'
                ) {
                    i+=4;
                    return JSON_TRUE;
                }
                else if( data.size() > (i + 4)
                        && data[i]      == 'f'
                        && data[i + 1]  == 'a'
                        && data[i + 2]  == 'l'
                        && data[i + 3]  == 's'
                        && data[i + 4]  == 'e'
                ) {
                    i+=5;
                    return JSON_FALSE;
                }
            }
            throw std::runtime_error(std::string("Expected JSON, got '") + data[i] + "' at pos " + std::to_string(i));
        }
        
        inline std::string indent(size_t depth)
        {
            return std::string(depth * 4, ' ');
        }
    }
    
    const char * JSON_Literal_Strings[] = 
    {
        "false",
        "true",
        "null",
    };
    
    std::string Element::to_json(bool trailing_comma,size_t depth) const
    {
        if(std::holds_alternative<int64_t>(data))
        {   //int
            return std::to_string(get_int());
        }
        else if(std::holds_alternative<double>(data))
        {   //double
            return std::to_string(get_double());
        }
        else if(std::holds_alternative<std::string>(data))
        {   //string
            return Util::QuoteString(get_str(),'"');
        }
        else if(std::holds_alternative<JSON_Literal>(data))
        {   //literal
            return JSON_Literal_Strings[size_t(std::get<JSON_Literal>(data))];
        }
        else if(std::holds_alternative<std::vector<Element>>(data))
        {   //array
            std::string s = "[\n";
            bool first = true;
            
            for(auto &e : get_arr())
            {
                if(!first) s += ",\n";
                
                s += indent(depth + 1);
                s += e.to_json(trailing_comma, depth + 1);
                
                first = false;
            }
            
            if(!get_arr().empty()) s += trailing_comma ? ",\n" : "\n";
            
            s += indent(depth) + "]";
            
            return s;
        }
        else if(std::holds_alternative<object_t>(data))
        {   //object
            std::string s = "{\n";
            bool first = true;
            
            for(auto &e : get_obj())
            {
                if(!first) s+= ",\n";
                
                s += indent(depth + 1);
                s += Util::QuoteString(e.first, '"');
                s += " : ";
                s += e.second.to_json(trailing_comma, depth + 1);
                
                first = false;
            }
            
            if(!get_obj().empty()) s += trailing_comma ? ",\n" : "\n";
            
            s += indent(depth) + "}";
            
            return s;
        }
        __builtin_unreachable();//all std::variant cases are handled in the if/else, this is absolutely unreachable
    }

    std::string Element::to_json_min() const
    {
        if(std::holds_alternative<int64_t>(data))
        {   //int
            return std::to_string(get_int());
        }
        else if(std::holds_alternative<double>(data))
        {   //double
            return std::to_string(get_double());
        }
        else if(std::holds_alternative<std::string>(data))
        {   //string
            return Util::QuoteString(get_str(), '"');
        }
        else if(std::holds_alternative<JSON_Literal>(data))
        {   //literal
            return JSON_Literal_Strings[size_t(std::get<JSON_Literal>(data))];
        }
        else if(std::holds_alternative<array_t>(data))
        {   //array
            std::string s = "[";
            bool first = true;
            
            for(auto &e : get_arr())
            {
                if(!first) s+=",";
                
                s += e.to_json_min();
                
                first = false;
            }
            
            s += "]";
            
            return s;
        }
        else if(std::holds_alternative<object_t>(data))
        {   //object
            std::string s = "{";
            bool first = true;
            
            for(auto &e : get_obj())
            {
                if(!first) s+= ",";
                
                s += Util::QuoteString(e.first, '"');
                s += ":";
                s += e.second.to_json_min();
                
                first = false;
            }
            
            s += "}";
            
            return s;
        }
        __builtin_unreachable();//all std::variant cases are handled in the if/else, this is absolutely unreachable
    }
    
    Element Parse(std::string_view data)
    {
        size_t i = 0;
        return get_element(data, i);
    }
    
}
