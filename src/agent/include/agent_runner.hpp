#pragma once

#include <boost/program_options.hpp>

#include <optional>

/// @brief The AgentRunner class is responsible for running the agent based on the provided command line arguments.
class AgentRunner
{
public:
    /// @brief Constructor for the AgentRunner class.
    /// @param argc The number of command line arguments.
    /// @param argv The command line arguments.
    AgentRunner(int argc, char* argv[]);

    /// @brief Runs the agent based on the provided command line arguments.
    /// @return The exit code of the agent.
    int Run() const;

private:
    /// @brief Set the available CLI options and parse the provided CLI arguments.
    /// @param argc The number of command line arguments.
    /// @param argv The command line arguments.
    void ParseOptions(int argc, char* argv[]);

    /// @brief Adds platform-specific options to the options description.
    void AddPlatformSpecificOptions();

    /// @brief Handles platform-specific options.
    /// @return std::nullopt if there were no platform-specific options to handle, their exit code otherwise.
    std::optional<int> HandlePlatformSpecificOptions() const;

    /// @brief Registers the agent with the provided command line arguments.
    /// @return 0 if the registration is successful, 1 otherwise.
    int RegisterAgent() const;

    /// @brief Displays the current status of the agent.
    void StatusAgent() const;

    /// @brief Starts the agent using the specified configuration file.
    /// @return 0 if the agent runs successfully, 1 otherwise.
    int StartAgent() const;

    boost::program_options::variables_map m_options;
    boost::program_options::options_description m_desc = {"Allowed options", 120};
};
