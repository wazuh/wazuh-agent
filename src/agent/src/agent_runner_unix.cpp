#include <agent_runner.hpp>

void AgentRunner::AddPlatformSpecificOptions() {}

std::optional<int> AgentRunner::HandlePlatformSpecificOptions() const
{
    return std::nullopt;
}
