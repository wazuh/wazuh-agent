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

    /// @brief Enrolls the agent with the provided command line arguments.
    /// @return 0 if the enrollment is successful, 1 otherwise.
    int EnrollAgent() const;

    /// @brief Displays the current status of the agent.
    void StatusAgent() const;

    /// @brief Starts the agent using the specified configuration file.
    /// @return 0 if the agent runs successfully, 1 otherwise.
    int StartAgent() const;

    /// @brief Reloads the agent configuration and all modules.
    /// @return 0 if the modules reload is successful, 1 otherwise.
    int ReloadModules() const;

    /// @brief Sends signal to the previous instance of the agent
    /// @param message The message to be sent
    /// @param configFilePath The path to the configuration file
    /// @return True if the signal was sent successfully
    bool SendSignal(const std::string& message, const std::string& configFilePath) const;

    boost::program_options::variables_map m_options;
    boost::program_options::options_description m_allOptions = {"Allowed options", 120};
    boost::program_options::options_description m_generalOptions = {"General options", 120};
    boost::program_options::options_description m_enrollmentOptions = {"Enrollment options", 120};
};
