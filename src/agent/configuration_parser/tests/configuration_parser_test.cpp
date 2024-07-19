#include <configuration_parser.hpp>

#include <gtest/gtest.h>

#include <map>
#include <memory>

using namespace configuration;

TEST(ConfigurationParser, GetConfigString)
{
    std::string strConfig = R"(
        [agent]
        manager_ip = "192.168.0.11"
        string_conf = "string"
    )";
    const auto parserStr = std::make_unique<configuration::ConfigurationParser>(strConfig);
    const auto ret = parserStr->GetConfig<std::string>("agent", "manager_ip");
    ASSERT_EQ(ret, "192.168.0.11");
}

TEST(ConfigurationParser, GetConfigArrayString)
{
    std::string strConfig = R"(
        [agent_array]
        array_manager_ip = ["192.168.0.0", "192.168.0.1"]
        string_conf = "string"
    )";
    const auto parserStr = std::make_unique<configuration::ConfigurationParser>(strConfig);
    const auto ret = parserStr->GetConfig<std::vector<std::string>>("agent_array", "array_manager_ip");
    ASSERT_EQ(ret[0], "192.168.0.0");
    ASSERT_EQ(ret[1], "192.168.0.1");
}

TEST(ConfigurationParser, GetConfigInt)
{
    std::string strConfig = R"(
        [agent_array]
        array_manager_ip = ["192.168.0.0", "192.168.0.1"]
        int_conf = 10
    )";
    const auto parserStr = std::make_unique<configuration::ConfigurationParser>(strConfig);
    const auto ret = parserStr->GetConfig<int>("agent_array", "int_conf");
    ASSERT_EQ(ret, 10);
}

TEST(ConfigurationParser, GetConfigFloat)
{
    std::string strConfig = R"(
        [agent_array]
        array_manager_ip = ["192.168.0.0", "192.168.0.1"]
        float_conf = 12.34
    )";
    const auto parserStr = std::make_unique<configuration::ConfigurationParser>(strConfig);
    const auto ret = parserStr->GetConfig<float>("agent_array", "float_conf");
    EXPECT_FLOAT_EQ(ret, 12.34);
}

TEST(ConfigurationParser, GetConfigNoKey)
{
    std::string strConfig = R"(
        [agent_array]
        array_manager_ip = ["192.168.0.0", "192.168.0.1"]
        float_conf = 12.34
    )";
    const auto parserStr = std::make_unique<configuration::ConfigurationParser>(strConfig);
    EXPECT_ANY_THROW(parserStr->GetConfig<float>("agent_array", "no_key"));
}

TEST(ConfigurationParser, GetConfigIntSubTable)
{
    std::string strConfig = R"(
        [agent_array]
        array_manager_ip = ["192.168.0.0", "192.168.0.1"]
        int_conf = 10
        [agent_array.sub_table]
        int_conf = 1234
    )";
    const auto parserStr = std::make_unique<configuration::ConfigurationParser>(strConfig);
    const auto ret = parserStr->GetConfig<int>("agent_array", "sub_table", "int_conf");
    ASSERT_EQ(ret, 1234);
}

TEST(ConfigurationParser, GetConfigBoolSubTable)
{
    std::string strConfig = R"(
        [agent_array]
        array_manager_ip = ["192.168.0.0", "192.168.0.1"]
        int_conf = 10
        [agent_array.sub_table]
        int_conf = 1234
        bool_conf = true
    )";
    const auto parserStr = std::make_unique<configuration::ConfigurationParser>(strConfig);
    const auto ret = parserStr->GetConfig<bool>("agent_array", "sub_table", "bool_conf");
    ASSERT_EQ(ret, true);
}

TEST(ConfigurationParser, GetConfigArrayMap)
{
    std::string strConfig = R"(
        [agent_array]
        array_manager_ip = ["192.168.0.0", "192.168.0.1"]
        string_conf = "string"
        api_auth = [{org_name = "dummy1", api_token = "api_token1"}, {org_name = "dummy2", api_token = "api_token2"}]
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
        [map_string]
        string_conf_1 = "string_1"
        string_conf_2 = "string_2"
    )";
    const auto parserStr = std::make_unique<configuration::ConfigurationParser>(strConfig);
    const auto ret = parserStr->GetConfig<std::map<std::string, std::string>>("map_string");
    ASSERT_EQ(ret.at("string_conf_1"), "string_1");
    ASSERT_EQ(ret.at("string_conf_2"), "string_2");
}

TEST(ConfigurationParser, GetConfigBadCast)
{
    std::string strConfig = R"(
        [bad_cast_array]
        string_conf_1 = "string_1"
        int_conf = 10
    )";
    const auto parserStr = std::make_unique<configuration::ConfigurationParser>(strConfig);
    EXPECT_ANY_THROW(parserStr->GetConfig<std::vector<std::string>>("bad_cast_array"));
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
