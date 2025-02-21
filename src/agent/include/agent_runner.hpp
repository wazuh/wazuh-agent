#pragma once

#include <agent.hpp>

#include <boost/program_options.hpp>

class AgentRunner
{
public:
    AgentRunner(int argc, char* argv[]);

    int Run() const;

private:
    void ParseOptions(int argc, char* argv[]);
    void AddPlatformSpecificOptions();

    boost::program_options::variables_map validOptions;
    boost::program_options::options_description cmdParser = {"Allowed options"};
};
