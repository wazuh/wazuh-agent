#pragma once

#include <optional>
#include <string>

namespace registration
{
    bool RegisterAgent(const std::string& user,
                       const std::string& password,
                       const std::optional<std::string>& name,
                       const std::optional<std::string>& ip);
} // namespace registration
