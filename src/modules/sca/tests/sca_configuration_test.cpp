#include <gtest/gtest.h>

#include <sca.hpp>

#include <configuration_parser.hpp>
#include <dbsync.hpp>

#include "mocks/mockdbsync.hpp"

#include <memory>
#include <string>

class ScaConfigurationTest : public ::testing::Test
{
protected:
    std::shared_ptr<configuration::ConfigurationParser> m_configurationParser = nullptr;
    SecurityConfigurationAssessment<MockDBSync>* m_sca = nullptr;
};

TEST_F(ScaConfigurationTest, ValidateExpectedConfigurationVariables)
{
    m_configurationParser = std::make_shared<configuration::ConfigurationParser>(std::string(R"(
        agent:
          path.data: test_path
        sca:
          enabled: true
          scan_on_start: true
          interval: 1h
          policies:
            - etc/shared/cis_debian10.yml
            - /my/custom/policy/path/my_policy.yaml
          policies_disabled:
            - ruleset/sca/cis_debian9.yml
    )"));
    m_sca = &SecurityConfigurationAssessment<MockDBSync>::Instance(m_configurationParser);
}
