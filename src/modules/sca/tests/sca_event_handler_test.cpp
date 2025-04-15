#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <sca_event_handler.hpp>

#include "mocks/sca_event_handler_mock.hpp"

using namespace sca_event_handler;

TEST(SCAEventHandlerTest, ProcessStateful_ValidInput)
{
    SCAEventHandlerMock handler;

    const nlohmann::json input = {
        {"check",
         {{"new",
           {{"id", "chk1"},
            {"title", "Ensure firewall is active"},
            {"description", "Verifies that the firewall is running"},
            {"rationale", "Security best practices"},
            {"status", "passed"}}}}},
        {"policy", {{"new", {{"id", "pol1"}, {"name", "CIS Ubuntu Benchmark"}, {"version", "1.1.0"}}}}},
        {"result", 2}};

    const nlohmann::json output = handler.ProcessStateful(input);

    ASSERT_TRUE(output.contains("event"));
    ASSERT_TRUE(output.contains("metadata"));

    auto event = output["event"];
    auto metadata = output["metadata"];

    EXPECT_EQ(event["check"]["id"], "chk1");
    EXPECT_EQ(event["policy"]["id"], "pol1");
    EXPECT_TRUE(event.contains("timestamp"));

    EXPECT_EQ(metadata["operation"], "create");
    EXPECT_EQ(metadata["module"], "sca");
    EXPECT_TRUE(metadata.contains("id"));
}

TEST(SCAEventHandlerTest, ProcessStateful_InvalidInput)
{
    SCAEventHandlerMock handler;

    const nlohmann::json input = {{"result", "invalid_result"}};

    const nlohmann::json output = handler.ProcessStateful(input);

    EXPECT_TRUE(output.empty());
}

TEST(SCAEventHandlerTest, ProcessStateless_ValidInput)
{
    SCAEventHandlerMock handler;
    const nlohmann::json input = {
        {"check", {{"new", {{"id", "chk1"}, {"result", "passed"}}}, {"old", {{"id", "chk1"}, {"result", "failed"}}}}},
        {"policy",
         {{"new", {{"id", "pol1"}, {"description", "Ensure firewall is active"}}},
          {"old", {{"id", "pol1"}, {"description", "Ensure firewall is running"}}}}},
        {"collector", "check"},
        {"result", 2}};
    const nlohmann::json output = handler.ProcessStateless(input);

    ASSERT_TRUE(output.contains("event"));
    ASSERT_TRUE(output.contains("metadata"));

    auto event = output["event"];
    auto metadata = output["metadata"];

    EXPECT_EQ(event["check"]["id"], "chk1");
    EXPECT_EQ(event["check"]["result"], "passed");
    EXPECT_EQ(event["check"]["previous"]["result"], "failed");

    EXPECT_EQ(event["policy"]["id"], "pol1");
    EXPECT_EQ(event["policy"]["description"], "Ensure firewall is active");
    EXPECT_EQ(event["policy"]["previous"]["description"], "Ensure firewall is running");

    ASSERT_EQ(event["event"]["changed_fields"].size(), 2);

    EXPECT_EQ(metadata["module"], "sca");
    EXPECT_EQ(metadata["collector"], "check");
}

TEST(SCAEventHandlerTest, ProcessStateless_InvalidInput)
{
    SCAEventHandlerMock handler;
    const nlohmann::json input = {{"policy", {{"new", {{"id", "pol1"}}}}}};

    const nlohmann::json output = handler.ProcessStateless(input);

    EXPECT_TRUE(output.empty());
}

TEST(SCAEventHandlerTest, CalculateHashId_ReturnsValidHash)
{
    SCAEventHandlerMock handler;
    const nlohmann::json data = {{"check", {{"id", "chk1"}}}, {"policy", {{"id", "pol1"}}}};

    const std::string hash = handler.CalculateHashId(data);
    EXPECT_FALSE(hash.empty());
}

TEST(SCAEventHandlerTest, CalculateHashId_MissingFields_Throws)
{
    SCAEventHandlerMock handler;
    const nlohmann::json data = {{"check", {}}, {"policy", {}}};

    EXPECT_THROW(handler.CalculateHashId(data), nlohmann::json::type_error);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
