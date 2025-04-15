#pragma once

#include <gmock/gmock.h>

#include <sca_event_handler.hpp>

#include "mockdbsync.hpp"

namespace sca_event_handler
{

    class SCAEventHandlerMock : public SCAEventHandler
    {
    public:
        SCAEventHandlerMock()
            : SCAEventHandler("agent-uuid", std::shared_ptr<MockDBSync>())
        {
        }

        std::string CalculateHashId(const nlohmann::json& data)
        {
            return SCAEventHandler::CalculateHashId(data);
        }

        nlohmann::json ProcessStateless(const nlohmann::json& event)
        {
            return SCAEventHandler::ProcessStateless(event);
        }

        nlohmann::json ProcessStateful(const nlohmann::json& event)
        {
            return SCAEventHandler::ProcessStateful(event);
        }
    };

} // namespace sca_event_handler
