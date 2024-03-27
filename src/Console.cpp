#include "Game.h"

namespace Game
{
    static void InternalAddConsoleLine(std::string_view text, std::span<const uint8_t> props)
    {
        if(GameConsoleOutput.size() == MaxConsoleLines)
        {
            GameConsoleOutput.erase(GameConsoleOutput.begin());
        }
        GameConsoleOutput.push_back({std::string(text), std::vector<uint8_t>(props.begin(), props.end())});
    }
}

void Game::ClearConsole()
{
    GameConsoleOutput.clear();
}

void Game::AddConsoleLine(std::string_view text)
{
    AddConsoleLine(text, {});
}

void Game::AddConsoleLine(std::string_view text, const std::vector<uint8_t> &_props)
{
    if(text.size() > 78 || _props.size() > 78)
    {
        std::span<const uint8_t> props(_props);
        
        std::vector<std::string_view> lines;
        std::vector<std::span<const uint8_t>> prop_lines;
        
        while(text.size() > 78)
        {
            lines.push_back(text.substr(0, 78));
            text = text.substr(78);
        }
        lines.push_back(text);
        
        while(props.size() > 78)
        {
            prop_lines.push_back(props.subspan(0, 78));
            props = props.subspan(78);
        }
        prop_lines.push_back(props);
        
        size_t n1 = lines.size();
        size_t n2 = prop_lines.size();
        
        size_t n = std::max(n1, n2);
        for(size_t i = 0; i < n; i++)
        {
            if(i < n1 && i < n2)
            {
                InternalAddConsoleLine(lines[i], prop_lines[i]);
            }
            else if(i < n1)
            {
                InternalAddConsoleLine(lines[i], {});
            }
            else if(i < n2)
            {
                InternalAddConsoleLine("", prop_lines[i]);
            }
        }
    }
    else
    {
        InternalAddConsoleLine(text, _props);
    }
}
