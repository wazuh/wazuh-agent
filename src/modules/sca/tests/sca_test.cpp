#include <gtest/gtest.h>

#include <sca.hpp>

#include <configuration_parser.hpp>
#include <dbsync.hpp>

#include <memory>
#include <string>

class MockDBSync
{
public:
    explicit MockDBSync(const HostType, const DbEngineType, const std::string&, const std::string&, const DbManagement)
    {
    }
};

// fixture
class ScaTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_configurationParser = std::make_shared<const configuration::ConfigurationParser>(std::string(R"(
            agent:
              retry_interval: 5
              verification_mode: none
            events:
              batch_size: 1
        )"));

        m_sca = std::make_unique<SecurityConfigurationAssessment<MockDBSync>>(m_configurationParser);
    }

    std::shared_ptr<const configuration::ConfigurationParser> m_configurationParser = nullptr;
    std::unique_ptr<SecurityConfigurationAssessment<MockDBSync>> m_sca = nullptr;
};

TEST_F(ScaTest, Constructor)
{
    SUCCEED();
}
