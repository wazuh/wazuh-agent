#pragma once

#include <command.hpp>

#include <tuple>

std::tuple<command_store::Status, std::string> DispatchCommand(const command_store::Command&);
