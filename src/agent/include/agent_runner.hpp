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

    int RegisterAgent() const;

    boost::program_options::variables_map m_options;
    boost::program_options::options_description m_desc = {"Allowed options", 120};
};
