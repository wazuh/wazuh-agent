#pragma once

#include <algorithm>
#include <string>
#include <vector>

class CommandlineParser
{
public:
    CommandlineParser(int& argc, char** argv)
    {
        for (int i = 1; i < argc; ++i)
        {
            m_tokens.push_back(std::string(argv[i]));
        }
    }

    const std::string& GetOptionValue(const std::string& option) const
    {
        auto itr = std::find(m_tokens.cbegin(), m_tokens.cend(), option);

        if (itr != m_tokens.cend() && ++itr != m_tokens.cend())
        {
            return *itr;
        }

        static const std::string empty_string("");
        return empty_string;
    }

    bool OptionExists(const std::string& option) const
    {
        auto bExists = std::find(m_tokens.begin(), m_tokens.end(), option) != m_tokens.end();
        return bExists;
    }

private:
    std::vector<std::string> m_tokens;
};
