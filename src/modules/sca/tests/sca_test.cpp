#include <gtest/gtest.h>

#include <sca.hpp>

#include <configuration_parser.hpp>
#include <dbsync.hpp>

#include <memory>
#include <string>

std::string MockDBSyncDbFilePath;

class MockDBSync
{
public:
    explicit MockDBSync(
        const HostType, const DbEngineType, const std::string& dbFilePath, const std::string&, const DbManagement)
    {
        MockDBSyncDbFilePath = dbFilePath;
    }
};

class ScaTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_configurationParser = std::make_shared<const configuration::ConfigurationParser>(std::string(R"(
            agent:
              retry_interval: 5
              verification_mode: none
              path.data: test_path
            events:
              batch_size: 1
        )"));

        m_sca = std::make_unique<SecurityConfigurationAssessment<MockDBSync>>(m_configurationParser);
    }

    std::shared_ptr<const configuration::ConfigurationParser> m_configurationParser = nullptr;
    std::unique_ptr<SecurityConfigurationAssessment<MockDBSync>> m_sca = nullptr;
};

TEST_F(ScaTest, ConstructorSetsDbFilePath)
{
    const auto expectedDbFilePath =
        m_configurationParser->GetConfigOrDefault(config::DEFAULT_DATA_PATH, "agent", "path.data");
    EXPECT_EQ(MockDBSyncDbFilePath, expectedDbFilePath + "/sca.db");
}

TEST_F(ScaTest, SetPushMessageFunctionStoresCallback)
{
    bool called = false;

    auto lambda = [&called](Message) -> int
    {
        called = true;
        return 123;
    };

    m_sca->SetPushMessageFunction(lambda);

    Message dummyMessage(MessageType::STATELESS, nlohmann::json {}, "dummyModule", "dummyType", "dummyMetaData");
    const int result = lambda(dummyMessage);

    EXPECT_TRUE(called);
    EXPECT_EQ(result, 123);
}
