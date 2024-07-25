#pragma once

#include <algorithm>
#include <string>
#include <vector>

class CommandlineParser
{
public:
    CommandlineParser(int& argc, char** argv)
    {
        for (int i = 1; i < argc; ++i) this->tokens.push_back(std::string(argv[i]));
    }

    const std::string& getOptionValue(const std::string& option) const
    {
        std::vector<std::string>::const_iterator itr;
        itr = std::find(this->tokens.begin(), this->tokens.end(), option);
        if (itr != this->tokens.end() && ++itr != this->tokens.end())
        {
            return *itr;
        }
        static const std::string empty_string("");
        return empty_string;
    }

    bool OptionExists(const std::string& option) const
    {
        bool bExists = std::find(this->tokens.begin(), this->tokens.end(), option) != this->tokens.end();
        return bExists;
    }

private:
    std::vector<std::string> tokens;
};
