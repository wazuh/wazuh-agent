#include <configuration_parser.hpp>

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>

using namespace configuration;

class ConfigurationParserFileTest : public ::testing::Test
{
protected:
    std::filesystem::path m_tempConfigFilePath;

    void SetUp() override
    {
        m_tempConfigFilePath = "temp_wazuh-agent.yml";

        std::ofstream outFile(m_tempConfigFilePath);
        outFile << R"(
            agent:
                server_url: https://myserver:28000
                registration_url: https://myserver:56000
            inventory:
                enabled: false
                interval: 7200
                scan_on_start: false
            logcollector:
                enabled: false
                localfiles:
                - /var/log/other.log
                reload_interval: 120
                file_wait: 1000
        )";
        outFile.close();
    }

    void TearDown() override
    {
        std::filesystem::remove(m_tempConfigFilePath);
    }
};

class ConfigurationParserInvalidYamlFileTest : public ::testing::Test
{
protected:
    std::filesystem::path m_tempConfigFilePath;

    void SetUp() override
    {
        m_tempConfigFilePath = "temp_wazuh-agent.yml";

        std::ofstream outFile(m_tempConfigFilePath);
        // This string does not respect the yaml format in the line of the file_wait field, the field is misaligned.
        // With this case we want it to fail when parsing the file.
        outFile << R"(
            agent:
                server_url: https://myserver:28000
                registration_url: https://myserver:56000
            inventory:
                enabled: false
                interval: 7200
                scan_on_start: false
            logcollector:
                enabled: false
                localfiles:
                - /var/log/other.log
                reload_interval: 120
                  file_wait: 1000
        )";
        outFile.close();
    }

    void TearDown() override
    {
        std::filesystem::remove(m_tempConfigFilePath);
    }
};

TEST(ConfigurationParser, GetConfigString)
{
    std::string strConfig = R"(
        agent:
          server_url: 192.168.0.11
          string_conf: string
    )";
    const auto parserStr = std::make_unique<configuration::ConfigurationParser>(strConfig);
    const auto ret = parserStr->GetConfig<std::string>("agent", "server_url");
    ASSERT_EQ(ret, "192.168.0.11");
}

TEST(ConfigurationParser, GetConfigArrayString)
{
    std::string strConfig = R"(
        agent_array:
          array_manager_ip:
            - 192.168.0.0
            - 192.168.0.1
          string_conf: string
    )";
    const auto parserStr = std::make_unique<configuration::ConfigurationParser>(strConfig);
    const auto ret = parserStr->GetConfig<std::vector<std::string>>("agent_array", "array_manager_ip");
    ASSERT_EQ(ret[0], "192.168.0.0");
    ASSERT_EQ(ret[1], "192.168.0.1");
}

TEST(ConfigurationParser, GetConfigInt)
{
    std::string strConfig = R"(
        agent_array:
          array_manager_ip:
            - 192.168.0.0
            - 192.168.0.1
          int_conf: 10
    )";
    const auto parserStr = std::make_unique<configuration::ConfigurationParser>(strConfig);
    const auto ret = parserStr->GetConfig<int>("agent_array", "int_conf");
    ASSERT_EQ(ret, 10);
}

TEST(ConfigurationParser, GetConfigFloat)
{
    std::string strConfig = R"(
        agent_array:
          array_manager_ip:
            - 192.168.0.0
            - 192.168.0.1
          float_conf: 12.34
    )";
    const auto parserStr = std::make_unique<configuration::ConfigurationParser>(strConfig);
    const auto ret = parserStr->GetConfig<float>("agent_array", "float_conf");
    EXPECT_FLOAT_EQ(ret, 12.34f);
}

TEST(ConfigurationParser, GetConfigNoKey)
{
    std::string strConfig = R"(
        agent_array:
          array_manager_ip:
            - 192.168.0.0
            - 192.168.0.1
          float_conf: 12.34
    )";
    const auto parserStr = std::make_unique<configuration::ConfigurationParser>(strConfig);
    EXPECT_ANY_THROW(parserStr->GetConfig<float>("agent_array", "no_key"));
}

TEST(ConfigurationParser, GetConfigIntSubTable)
{
    std::string strConfig = R"(
        agent_array:
          array_manager_ip:
            - 192.168.0.0
            - 192.168.0.1
          int_conf: 10
          sub_table:
            int_conf: 1234
    )";
    const auto parserStr = std::make_unique<configuration::ConfigurationParser>(strConfig);
    const auto ret = parserStr->GetConfig<int>("agent_array", "sub_table", "int_conf");
    ASSERT_EQ(ret, 1234);
}

TEST(ConfigurationParser, GetConfigBoolSubTable)
{
    std::string strConfig = R"(
        agent_array:
          array_manager_ip:
            - 192.168.0.0
            - 192.168.0.1
          int_conf: 10
          sub_table:
            int_conf: 1234
            bool_conf: true
    )";
    const auto parserStr = std::make_unique<configuration::ConfigurationParser>(strConfig);
    const auto ret = parserStr->GetConfig<bool>("agent_array", "sub_table", "bool_conf");
    ASSERT_EQ(ret, true);
}

TEST(ConfigurationParser, GetConfigArrayMap)
{
    std::string strConfig = R"(
        agent_array:
          array_manager_ip:
            - 192.168.0.0
            - 192.168.0.1
          string_conf: string
          api_auth:
            - org_name: dummy1
              api_token: api_token1
            - org_name: dummy2
              api_token: api_token2
    )";
    const auto parserStr = std::make_unique<configuration::ConfigurationParser>(strConfig);
    const auto ret = parserStr->GetConfig<std::vector<std::map<std::string, std::string>>>("agent_array", "api_auth");
    ASSERT_EQ(ret[0].at("org_name"), "dummy1");
    ASSERT_EQ(ret[0].at("api_token"), "api_token1");
    ASSERT_EQ(ret[1].at("org_name"), "dummy2");
    ASSERT_EQ(ret[1].at("api_token"), "api_token2");
}

TEST(ConfigurationParser, GetConfigMap)
{
    std::string strConfig = R"(
        map_string:
          string_conf_1: string_1
          string_conf_2: string_2
    )";
    const auto parserStr = std::make_unique<configuration::ConfigurationParser>(strConfig);
    const auto ret = parserStr->GetConfig<std::map<std::string, std::string>>("map_string");
    ASSERT_EQ(ret.at("string_conf_1"), "string_1");
    ASSERT_EQ(ret.at("string_conf_2"), "string_2");
}

TEST(ConfigurationParser, GetConfigBadCast)
{
    std::string strConfig = R"(
        bad_cast_array:
          string_conf_1: string_1
          int_conf: 10
    )";
    const auto parserStr = std::make_unique<configuration::ConfigurationParser>(strConfig);
    EXPECT_ANY_THROW(parserStr->GetConfig<std::vector<std::string>>("bad_cast_array"));
}

TEST(ConfigurationParser, GetConfigMultiNode)
{
    std::string strConfig = R"(
        agent_array:
          array_manager_ip:
            - 192.168.0.0
            - 192.168.0.1
          string_conf: string
        logcollector:
          enabled: true
          localfiles:
          - /var/log/auth.log
          - /var/log/other.log
          reload_interval: 60
          file_wait: 500
    )";
    const auto parserStr = std::make_unique<configuration::ConfigurationParser>(strConfig);
    const auto ret = parserStr->GetConfig<std::vector<std::string>>("agent_array", "array_manager_ip");
    const auto retEnabled = parserStr->GetConfig<bool>("logcollector", "enabled");
    const auto retFileWait = parserStr->GetConfig<int>("logcollector", "file_wait");
    const auto retLocalFiles = parserStr->GetConfig<std::vector<std::string>>("logcollector", "localfiles");
    ASSERT_EQ(ret[0], "192.168.0.0");
    ASSERT_EQ(ret[1], "192.168.0.1");
    ASSERT_TRUE(retEnabled);
    ASSERT_EQ(retLocalFiles[0], "/var/log/auth.log");
    ASSERT_EQ(retLocalFiles[1], "/var/log/other.log");
    ASSERT_EQ(retFileWait, 500);
}

TEST(ConfigurationParser, ConfigurationParserStringMisaligned)
{
    std::string strConfig = R"(
        users:
          - name: Alice
           - name: Bob
    )";
    EXPECT_THROW(std::make_unique<configuration::ConfigurationParser>(strConfig), std::exception);
}

TEST_F(ConfigurationParserFileTest, ValidConfigFileLoadsCorrectly)
{
    try
    {
        const auto parser = std::make_unique<configuration::ConfigurationParser>(m_tempConfigFilePath);

        EXPECT_EQ(parser->GetConfig<std::string>("agent", "server_url"), "https://myserver:28000");
        EXPECT_FALSE(parser->GetConfig<bool>("inventory", "enabled"));
        EXPECT_EQ(parser->GetConfig<int>("inventory", "interval"), 7200);
        EXPECT_FALSE(parser->GetConfig<bool>("logcollector", "enabled"));
        EXPECT_EQ(parser->GetConfig<int>("logcollector", "file_wait"), 1000);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        std::filesystem::remove(m_tempConfigFilePath);
        throw;
    }
}

TEST_F(ConfigurationParserInvalidYamlFileTest, InvalidConfigFileLoadsDefault)
{
    try
    {
        const auto parser = std::make_unique<configuration::ConfigurationParser>(m_tempConfigFilePath);

        EXPECT_EQ(parser->GetConfig<std::string>("agent", "server_url"), "https://localhost:27000");
        EXPECT_TRUE(parser->GetConfig<bool>("inventory", "enabled"));
        EXPECT_EQ(parser->GetConfig<int>("inventory", "interval"), 3600);
        EXPECT_TRUE(parser->GetConfig<bool>("logcollector", "enabled"));
        EXPECT_EQ(parser->GetConfig<int>("logcollector", "file_wait"), 500);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        std::filesystem::remove(m_tempConfigFilePath);
        throw;
    }
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
