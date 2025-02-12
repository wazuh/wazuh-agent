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
                read_interval: 1000
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
        // This string does not respect the yaml format in the line of the read_interval field, the field is misaligned.
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
                  read_interval: 1000
        )";
        outFile.close();
    }

    void TearDown() override
    {
        std::filesystem::remove(m_tempConfigFilePath);
    }
};

// NOLINTBEGIN(bugprone-unchecked-optional-access)
TEST(ConfigurationParser, GetConfigString)
{
    std::string strConfig = R"(
        agent:
          server_url: 192.168.0.11
          string_conf: string
    )";
    const auto parserStr = std::make_unique<configuration::ConfigurationParser>(strConfig);
    const auto ret = parserStr->GetConfig<std::string>("agent", "server_url").value();
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
    const auto ret = parserStr->GetConfig<std::vector<std::string>>("agent_array", "array_manager_ip").value();
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
    const auto ret = parserStr->GetConfig<int>("agent_array", "int_conf").value();
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
    const auto ret = ParseTimeUnit(parserStr->GetConfig<std::string>("agent_array", "milliseconds_conf").value());
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
    const auto ret = ParseTimeUnit(parserStr->GetConfig<std::string>("agent_array", "seconds_conf").value());
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
    const auto ret = ParseTimeUnit(parserStr->GetConfig<std::string>("agent_array", "minutes_conf").value());
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
    const auto ret = ParseTimeUnit(parserStr->GetConfig<std::string>("agent_array", "hours_conf").value());
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
    const auto ret = ParseTimeUnit(parserStr->GetConfig<std::string>("agent_array", "days_conf").value());
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
    EXPECT_ANY_THROW(ParseTimeUnit(parserStr->GetConfig<std::string>("agent_array", "time_invalid_conf").value()));
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
    const auto ret = parserStr->GetConfig<float>("agent_array", "float_conf").value();
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
    EXPECT_EQ(parserStr->GetConfig<float>("agent_array", "no_key"), std::nullopt);
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
    const auto ret = parserStr->GetConfig<int>("agent_array", "sub_table", "int_conf").value();
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
    const auto ret = parserStr->GetConfig<bool>("agent_array", "sub_table", "bool_conf").value();
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
    const auto ret =
        parserStr->GetConfig<std::vector<std::map<std::string, std::string>>>("agent_array", "api_auth").value();
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
    const auto ret = parserStr->GetConfig<std::map<std::string, std::string>>("map_string").value();
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
    EXPECT_EQ(parserStr->GetConfig<std::vector<std::string>>("bad_cast_array"), std::nullopt);
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
          read_interval: 500
    )";
    const auto parserStr = std::make_unique<configuration::ConfigurationParser>(strConfig);
    const auto ret = parserStr->GetConfig<std::vector<std::string>>("agent_array", "array_manager_ip").value();
    const auto retEnabled = parserStr->GetConfig<bool>("logcollector", "enabled").value();
    const auto retFileWait = parserStr->GetConfig<int>("logcollector", "read_interval").value();
    const auto retLocalFiles = parserStr->GetConfig<std::vector<std::string>>("logcollector", "localfiles").value();
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

        EXPECT_EQ(parser->GetConfig<std::string>("agent", "server_url").value(), "https://myserver:28000");
        EXPECT_FALSE(parser->GetConfig<bool>("inventory", "enabled").value());
        EXPECT_EQ(parser->GetConfig<int>("inventory", "interval").value(), 7200);
        EXPECT_FALSE(parser->GetConfig<bool>("logcollector", "enabled").value());
        EXPECT_EQ(parser->GetConfig<int>("logcollector", "read_interval").value(), 1000);
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

TEST(ConfigurationParser, GetConfigBytes)
{
    // Config should contain batch_size string in order to apply parsing
    std::string strConfig = R"(
        batch_size:
          size_bytes: 500B
          size_KB: 45KB
          size_MB: 1MB
          size_M: 4M
          size_GB: 2GB
          size_G: 3G
          size_default_KB: 53
    )";
    const auto parserStr = std::make_unique<configuration::ConfigurationParser>(strConfig);
    const auto ret = ParseSizeUnit(parserStr->GetConfig<std::string>("batch_size", "size_bytes").value());
    ASSERT_EQ(ret, 500);
    const auto retKB = ParseSizeUnit(parserStr->GetConfig<std::string>("batch_size", "size_KB").value());
    ASSERT_EQ(retKB, 45000);
    const auto retMB = ParseSizeUnit(parserStr->GetConfig<std::string>("batch_size", "size_MB").value());
    ASSERT_EQ(retMB, 1000000);
    const auto retM = ParseSizeUnit(parserStr->GetConfig<std::string>("batch_size", "size_M").value());
    ASSERT_EQ(retM, 4000000);
    const auto retGB = ParseSizeUnit(parserStr->GetConfig<std::string>("batch_size", "size_GB").value());
    ASSERT_EQ(retGB, 2000000000);
    const auto retG = ParseSizeUnit(parserStr->GetConfig<std::string>("batch_size", "size_G").value());
    ASSERT_EQ(retG, 3000000000);
    const auto retDefaultKB = ParseSizeUnit(parserStr->GetConfig<std::string>("batch_size", "size_default_KB").value());
    ASSERT_EQ(retDefaultKB, 53);
}

TEST(ConfigurationParser, GetConfigOrDefaultReturnsDefaultIfKeyNotFound)
{
    const std::string strConfig = R"(
        agent:
            server_url: https://myserver:28000
    )";
    const auto configParser = std::make_unique<configuration::ConfigurationParser>(strConfig);
    const auto ret = configParser->GetConfigOrDefault("default", "agent", "key_not_found");
    EXPECT_EQ(ret, "default");
}

TEST(ConfigurationParser, GetConfigOrDefaultReturnsExpectedValue)
{
    const std::string strConfig = R"(
        agent:
            server_url: https://myserver:28000
    )";
    const auto configParser = std::make_unique<configuration::ConfigurationParser>(strConfig);
    const auto ret = configParser->GetConfigOrDefault("default", "agent", "server_url");
    EXPECT_EQ(ret, "https://myserver:28000");
}

TEST(ConfigurationParser, GetConfigInRangeOrDefaultReturnsDefaultIfKeyNotFound)
{
    const std::string strConfig = R"(
        agent:
            some_value: 13
            some_size: 26KB
            some_time: 39ms
    )";
    const auto configParser = std::make_unique<configuration::ConfigurationParser>(strConfig);

    const auto expectedValue = configParser->GetConfigInRangeOrDefault(
        42, std::optional<int>(0), std::optional<int>(100), "agent", "key_not_found");
    EXPECT_EQ(expectedValue, 42);

    const size_t expectedSize = configParser->GetBytesConfigInRangeOrDefault("42B", 0, 100, "agent", "key_not_found");
    EXPECT_EQ(expectedSize, 42);

    const time_t expectedTime = configParser->GetTimeConfigInRangeOrDefault("42ms", 0, 100, "agent", "key_not_found");
    EXPECT_EQ(expectedTime, 42);
}

TEST(ConfigurationParser, GetConfigInRangeOrDefaultReturnsDefaultIfRangeIsInvalid)
{
    const std::string strConfig = R"(
        agent:
            some_value: 13
            some_size: 26KB
            some_time: 39ms
    )";
    const auto configParser = std::make_unique<configuration::ConfigurationParser>(strConfig);

    const auto expectedValue = configParser->GetConfigInRangeOrDefault(
        42, std::optional<int>(1), std::optional<int>(0), "agent", "some_value");
    EXPECT_EQ(expectedValue, 42);

    const size_t expectedSize = configParser->GetBytesConfigInRangeOrDefault("42B", 1, 0, "agent", "some_size");
    EXPECT_EQ(expectedSize, 42);

    const time_t expectedTime = configParser->GetTimeConfigInRangeOrDefault("42ms", 1, 0, "agent", "some_time");
    EXPECT_EQ(expectedTime, 42);
}

TEST(ConfigurationParser, GetConfigInRangeOrDefaultReturnsDefaultIfValueIsOutOfRange)
{
    const std::string strConfig = R"(
        agent:
            some_value: 13
            some_size: 26KB
            some_time: 39ms
    )";
    const auto configParser = std::make_unique<configuration::ConfigurationParser>(strConfig);

    const auto expectedDefaultSinceValueIsAboveRange = configParser->GetConfigInRangeOrDefault(
        42, std::optional<int>(0), std::optional<int>(1), "agent", "some_value");
    EXPECT_EQ(expectedDefaultSinceValueIsAboveRange, 42);

    const auto expectedDefaultSinceValueIsBelowRange = configParser->GetConfigInRangeOrDefault(
        42, std::optional<int>(14), std::optional<int>(15), "agent", "some_value");
    EXPECT_EQ(expectedDefaultSinceValueIsBelowRange, 42);

    const size_t expectedDefaultSinceSizeIsAboveRange =
        configParser->GetBytesConfigInRangeOrDefault("42B", 0, 1, "agent", "some_size");
    EXPECT_EQ(expectedDefaultSinceSizeIsAboveRange, 42);

    const size_t expectedDefaultSinceSizeIsBelowRange =
        configParser->GetBytesConfigInRangeOrDefault("42B", 27000, 28000, "agent", "some_size");
    EXPECT_EQ(expectedDefaultSinceSizeIsBelowRange, 42);

    const time_t expectedDefaultSinceTimeIsAboveRange =
        configParser->GetTimeConfigInRangeOrDefault("42ms", 0, 1, "agent", "some_time");
    EXPECT_EQ(expectedDefaultSinceTimeIsAboveRange, 42);

    const time_t expectedDefaultSinceTimeIsBelowRange =
        configParser->GetTimeConfigInRangeOrDefault("42ms", 40, 41, "agent", "some_time");
    EXPECT_EQ(expectedDefaultSinceTimeIsBelowRange, 42);
}

TEST(ConfigurationParser, GetConfigInRangeOrDefaultReturnsValueIfExistsAndWithinRange)
{
    const std::string strConfig = R"(
        agent:
            some_value: 13
            some_size: 26KB
            some_time: 39ms
    )";
    const auto configParser = std::make_unique<configuration::ConfigurationParser>(strConfig);
    const auto expectedValue = configParser->GetConfigInRangeOrDefault(
        42, std::optional<int>(12), std::optional<int>(14), "agent", "some_value");
    EXPECT_EQ(expectedValue, 13);

    const size_t expectedSize = configParser->GetBytesConfigInRangeOrDefault("42B", 25000, 27000, "agent", "some_size");
    EXPECT_EQ(expectedSize, 26000);

    const time_t expectedTime = configParser->GetTimeConfigInRangeOrDefault("42ms", 38, 40, "agent", "some_time");
    EXPECT_EQ(expectedTime, 39);
}

// NOLINTEND(bugprone-unchecked-optional-access)

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
