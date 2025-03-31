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

TEST(ScaTest, Constructor)
{
    const auto configParser = std::make_shared<const configuration::ConfigurationParser>(std::string(R"(
        agent:
          retry_interval: 5
          verification_mode: none
        events:
          batch_size: 1
    )"));

    const SecurityConfigurationAssessment<MockDBSync> sca(configParser);
}
