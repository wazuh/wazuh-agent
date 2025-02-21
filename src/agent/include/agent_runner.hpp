#pragma once

#include <agent.hpp>

#include <boost/program_options.hpp>

#include <optional>

class AgentRunner
{
public:
    AgentRunner(int argc, char* argv[]);

    int Run() const;

private:
    void ParseOptions(int argc, char* argv[]);
    void AddPlatformSpecificOptions();

    std::optional<int> HandlePlatformSpecificOptions() const;

    /// @brief Registers the agent with the provided command line arguments.
    /// @return 0 if the registration is successful, 1 otherwise.
    int RegisterAgent() const;

    boost::program_options::variables_map m_options;
    boost::program_options::options_description m_desc = {"Allowed options", 120};
};
