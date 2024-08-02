#pragma once

#include <optional>
#include <string>

bool RegisterAgent(const std::string& user,
                   const std::string& password,
                   const std::optional<std::string>& name,
                   const std::optional<std::string>& ip);
