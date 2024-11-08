#pragma once

#include <algorithm>
#include <string>
#include <vector>

/// @brief A class for parsing the command line arguments
///
/// This class is used to extract values associated with command line options.
class CommandlineParser
{
public:
    /// @brief Constructor
    /// @param argc The number of command line arguments
    /// @param argv The command line arguments
    CommandlineParser(int argc, char** argv)
    {
        for (int i = 1; i < argc; ++i)
        {
            m_tokens.push_back(std::string(argv[i]));
        }
    }

    /// @brief Returns the value associated with the given command line option
    /// @param option The command line option
    /// @return The value associated with the given command line option
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

    /// @brief Checks if the given command line option exists
    /// @param option The command line option
    /// @return True if the given command line option exists, false otherwise
    bool OptionExists(const std::string& option) const
    {
        auto bExists = std::find(m_tokens.begin(), m_tokens.end(), option) != m_tokens.end();
        return bExists;
    }

private:
    /// @brief Vector of command line tokens
    std::vector<std::string> m_tokens;
};
