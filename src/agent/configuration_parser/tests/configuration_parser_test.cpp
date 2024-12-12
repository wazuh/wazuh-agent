#include <configuration_parser.hpp>

#include <config.h>
#include <gtest/gtest.h>

#include <ctime>
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
    const auto ret = parserStr->GetConfig<std::string>("agent", "server_url").value_or("Invalid string");
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
    const auto ret = parserStr->GetConfig<std::vector<std::string>>("agent_array", "array_manager_ip")
                         .value_or(std::vector<std::string>({"Invalid string1", "Invalid string2"}));
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
    const auto ret = parserStr->GetConfig<int>("agent_array", "int_conf").value_or(1234);
    ASSERT_EQ(ret, 10);
}

TEST(ConfigurationParser, GetConfigMilliseconds)
{
    std::string strConfig = R"(
        agent_array:
          array_manager_ip:
            - 192.168.0.0
            - 192.168.0.1
          milliseconds_conf: 500ms
    )";
    const auto parserStr = std::make_unique<configuration::ConfigurationParser>(strConfig);
    const auto ret = parserStr->GetConfig<std::time_t>("agent_array", "milliseconds_conf").value_or(1234);
    ASSERT_EQ(ret, 500);
}

TEST(ConfigurationParser, GetConfigSeconds)
{
    std::string strConfig = R"(
        agent_array:
          array_manager_ip:
            - 192.168.0.0
            - 192.168.0.1
          seconds_conf: 45s
    )";
    const auto parserStr = std::make_unique<configuration::ConfigurationParser>(strConfig);
    const auto ret = parserStr->GetConfig<std::time_t>("agent_array", "seconds_conf").value_or(1234);
    ASSERT_EQ(ret, 45000);
}

TEST(ConfigurationParser, GetConfigMinutes)
{
    std::string strConfig = R"(
        agent_array:
          array_manager_ip:
            - 192.168.0.0
            - 192.168.0.1
          minutes_conf: 3m
    )";
    const auto parserStr = std::make_unique<configuration::ConfigurationParser>(strConfig);
    const auto ret = parserStr->GetConfig<std::time_t>("agent_array", "minutes_conf").value_or(1234);
    ASSERT_EQ(ret, 180000);
}

TEST(ConfigurationParser, GetConfigHours)
{
    std::string strConfig = R"(
        agent_array:
          array_manager_ip:
            - 192.168.0.0
            - 192.168.0.1
          hours_conf: 2h
    )";
    const auto parserStr = std::make_unique<configuration::ConfigurationParser>(strConfig);
    const auto ret = parserStr->GetConfig<std::time_t>("agent_array", "hours_conf").value_or(1234);
    ASSERT_EQ(ret, 7200000);
}

TEST(ConfigurationParser, GetConfigDays)
{
    std::string strConfig = R"(
        agent_array:
          array_manager_ip:
            - 192.168.0.0
            - 192.168.0.1
          days_conf: 1d
    )";
    const auto parserStr = std::make_unique<configuration::ConfigurationParser>(strConfig);
    const auto ret = parserStr->GetConfig<std::time_t>("agent_array", "days_conf").value_or(1234);
    ASSERT_EQ(ret, 86400000);
}

TEST(ConfigurationParser, GetConfigTimeInvalid)
{
    std::string strConfig = R"(
        agent_array:
          array_manager_ip:
            - 192.168.0.0
            - 192.168.0.1
          time_invalid_conf: 30k
    )";
    const auto parserStr = std::make_unique<configuration::ConfigurationParser>(strConfig);
    const auto ret = parserStr->GetConfig<std::time_t>("agent_array", "time_invalid_conf").value_or(1234);
    ASSERT_EQ(ret, 1234);
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
    const auto ret = parserStr->GetConfig<float>("agent_array", "float_conf").value_or(1.1f);
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
    const auto ret = parserStr->GetConfig<float>("agent_array", "no_key").value_or(1.1f); // NOLINT
    EXPECT_FLOAT_EQ(ret, 1.1f);
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
    const auto ret = parserStr->GetConfig<int>("agent_array", "sub_table", "int_conf").value_or(0);
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
    const auto ret = parserStr->GetConfig<bool>("agent_array", "sub_table", "bool_conf").value_or(false);
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
    const auto ret = parserStr->GetConfig<std::vector<std::map<std::string, std::string>>>("agent_array", "api_auth")
                         .value_or(std::vector<std::map<std::string, std::string>> {
                             {{"org_name", "default1"}, {"api_token", "default_token1"}},
                             {{"org_name", "default2"}, {"api_token", "default_token2"}}});
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
    const auto ret = parserStr->GetConfig<std::map<std::string, std::string>>("map_string")
                         .value_or(std::map<std::string, std::string> {{"string_conf_1", "default_1"},
                                                                       {"string_conf_2", "default_2"}});
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
    const auto ret = parserStr->GetConfig<std::vector<std::string>>("bad_cast_array")
                         .value_or(std::vector<std::string> {"dummy", "string"});
    ASSERT_EQ(ret[0], "dummy");
    ASSERT_EQ(ret[1], "string");
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
    const auto ret = parserStr->GetConfig<std::vector<std::string>>("agent_array", "array_manager_ip")
                         .value_or(std::vector<std::string> {});
    const auto retEnabled = parserStr->GetConfig<bool>("logcollector", "enabled").value_or(false);
    const auto retFileWait = parserStr->GetConfig<int>("logcollector", "file_wait").value_or(0);
    const auto retLocalFiles = parserStr->GetConfig<std::vector<std::string>>("logcollector", "localfiles")
                                   .value_or(std::vector<std::string> {});
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

        EXPECT_EQ(parser->GetConfig<std::string>("agent", "server_url").value_or(""), "https://myserver:28000");
        EXPECT_FALSE(parser->GetConfig<bool>("inventory", "enabled").value_or(true));
        EXPECT_EQ(parser->GetConfig<int>("inventory", "interval").value_or(0), 7200);
        EXPECT_FALSE(parser->GetConfig<bool>("logcollector", "enabled").value_or(true));
        EXPECT_EQ(parser->GetConfig<int>("logcollector", "file_wait").value_or(0), 1000);
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

        EXPECT_EQ(parser->GetConfig<std::string>("agent", "server_url").value_or("https://localhost:27000"),
                  "https://localhost:27000");
        EXPECT_TRUE(parser->GetConfig<bool>("inventory", "enabled").value_or(true));
        EXPECT_EQ(parser->GetConfig<int>("inventory", "interval").value_or(3600), 3600);
        EXPECT_TRUE(parser->GetConfig<bool>("logcollector", "enabled").value_or(true));
        EXPECT_EQ(parser->GetConfig<int>("logcollector", "file_wait").value_or(500), 500);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        std::filesystem::remove(m_tempConfigFilePath);
        throw;
    }
}

TEST_F(ConfigurationParserInvalidYamlFileTest, isValidYamlFileInvalid)
{
    try
    {
        const auto parser = std::make_unique<configuration::ConfigurationParser>();

        EXPECT_FALSE(parser->isValidYamlFile(m_tempConfigFilePath));
    }
    catch (const std::exception&)
    {
        std::filesystem::remove(m_tempConfigFilePath);
        throw;
    }
}

TEST_F(ConfigurationParserFileTest, isValidYamlFileValid)
{
    try
    {
        const auto parser = std::make_unique<configuration::ConfigurationParser>();

        EXPECT_TRUE(parser->isValidYamlFile(m_tempConfigFilePath));
    }
    catch (const std::exception&)
    {
        std::filesystem::remove(m_tempConfigFilePath);
        throw;
    }
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
