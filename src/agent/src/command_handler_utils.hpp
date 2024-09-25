#pragma once

#include <command.hpp>

#include <moduleWrapper.hpp>
#include <multitype_queue.hpp>

#include <memory>
#include <tuple>

std::tuple<command_store::Status, std::string> DispatchCommand(const command_store::CommandEntry& commandEntry,
                                                               std::shared_ptr<ModuleWrapper> module,
                                                               std::shared_ptr<IMultiTypeQueue> messageQueue);
