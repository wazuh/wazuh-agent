#include <gtest/gtest.h>

#include <sca_policy_parser.hpp>

#include <string>

namespace
{
    YAML::Node LoadFromString(const std::string& yml)
    {
        return YAML::Load(yml);
    }
} // namespace

TEST(PolicyParserTest, InvalidYamlFileNotThrows)
{
    const auto invalidYamlLoader = [](const std::string&) -> YAML::Node
    {
        throw YAML::Exception(YAML::Mark::null_mark(), "Invalid YAML");
    };

    EXPECT_NO_THROW({ const PolicyParser parser("dummy.yaml", invalidYamlLoader); });
}

TEST(PolicyParserTest, YamlSequenceIsValid)
{
    const auto sequenceYamlLoader = [](const std::string&) -> YAML::Node
    {
        return YAML::Load("- item1\n- item2");
    };

    const PolicyParser parser("dummy.yaml", sequenceYamlLoader);
    EXPECT_TRUE(parser.IsValidYamlFile("dummy.yaml")); // Sequences are valid top-level YAML
}

TEST(PolicyParserTest, YamlMapIsValid)
{
    const auto mapYamlLoader = [](const std::string&) -> YAML::Node
    {
        return YAML::Load("key: value");
    };

    const PolicyParser parser("dummy.yaml", mapYamlLoader);
    EXPECT_TRUE(parser.IsValidYamlFile("dummy.yaml")); // Maps are valid top-level YAML
}

TEST(PolicyParserTest, InvalidYamlStructureReturnsFalse)
{
    const auto invalidYamlLoader = [](const std::string&) -> YAML::Node
    {
        YAML::Node node;
        node.reset();
        return node;
    };

    const PolicyParser parser("dummy.yaml", invalidYamlLoader);
    EXPECT_FALSE(parser.IsValidYamlFile("dummy.yaml"));
}

TEST(PolicyParserTest, ConstructorExtractsVariables)
{
    const std::string yml = R"(
      variables:
        $var1: /etc
        $var11: /usr
      policy:
        id: policy1
      checks:
        - id: check1
          title: "title"
          condition: "all"
          rules:
            - 'f: $var1/passwd exists'
            - 'f: $var11/shared exists'
      )";

    const auto loader = [yml](const std::string&)
    {
        return LoadFromString(yml);
    };

    const PolicyParser parser("dummy.yaml", loader);

    nlohmann::json j;
    const auto policyOpt = parser.ParsePolicy(j);

    ASSERT_TRUE(policyOpt.has_value());
    ASSERT_EQ(j["checks"].size(), 1);
    EXPECT_EQ(j["checks"][0]["id"], "check1");
    EXPECT_EQ(j["checks"][0]["title"], "title");
    EXPECT_EQ(j["checks"][0]["condition"], "all");
    EXPECT_EQ(j["checks"][0]["rules"], "f: /etc/passwd exists, f: /usr/shared exists");

    ASSERT_EQ(j["policies"].size(), 1);
    EXPECT_EQ(j["policies"][0]["id"], "policy1");
}

TEST(PolicyParserTest, MissingPolicyReturnsNullopt)
{
    const std::string yml = R"(
      checks:
        - id: check1
          title: Title
          condition: all
          rules: ['f: /test exists']
      )";

    const auto loader = [yml](const std::string&)
    {
        return LoadFromString(yml);
    };

    const PolicyParser parser("dummy.yaml", loader);
    nlohmann::json j;
    const auto policyOpt = parser.ParsePolicy(j);
    EXPECT_FALSE(policyOpt.has_value());
}

TEST(PolicyParserTest, EmptyRequirementsReturnsNullopt)
{
    const std::string yml = R"(
      policy:
        id: test_policy
      requirements:
        title: "req title"
      )";

    const auto loader = [yml](const std::string&)
    {
        return LoadFromString(yml);
    };

    const PolicyParser parser("dummy.yaml", loader);
    nlohmann::json j;
    const auto policyOpt = parser.ParsePolicy(j);
    EXPECT_FALSE(policyOpt.has_value());
}

TEST(PolicyParserTest, MissingChecksReturnsNullopt)
{
    const std::string yml = R"(
      policy:
        id: policy_id
      requirements:
        title: title
        condition: all
        rules: ['f: /etc/passwd exists']
      )";

    const auto loader = [yml](const std::string&)
    {
        return LoadFromString(yml);
    };

    const PolicyParser parser("dummy.yaml", loader);
    nlohmann::json j;
    const auto policyOpt = parser.ParsePolicy(j);
    EXPECT_FALSE(policyOpt.has_value());
}

TEST(PolicyParserTest, InvalidConditionReturnsNullopt)
{
    const std::string yml = R"(
      policy:
        id: policy_id
      requirements:
        title: title
        condition: invalid_condition
        rules: ['f: /etc/passwd exists']
      )";

    const auto loader = [yml](const std::string&)
    {
        return LoadFromString(yml);
    };

    const PolicyParser parser("dummy.yaml", loader);
    nlohmann::json j;
    const auto policyOpt = parser.ParsePolicy(j);
    EXPECT_FALSE(policyOpt.has_value());
}

TEST(PolicyParserTest, InvalidRuleIsHandledGracefully)
{
    const std::string yml = R"(
      policy:
        id: policy_id
      checks:
        - id: "check1"
          title: "Title"
          condition: any
          rules:
            - "invalid_rule"
      )";

    const auto loader = [yml](const std::string&)
    {
        return LoadFromString(yml);
    };

    const PolicyParser parser("dummy.yaml", loader);
    nlohmann::json j;
    const auto policyOpt = parser.ParsePolicy(j);

    ASSERT_TRUE(policyOpt.has_value());
    ASSERT_EQ(j["checks"].size(), 1);
    EXPECT_EQ(j["checks"][0]["rules"], "");
}

TEST(PolicyParserTest, YamlNodeToJsonParsesMapWithSequenceValues)
{
    const std::string yml = R"(
      policy:
        id: policy_1
      checks:
        - id: "check1"
          title: "Complex check"
          condition: any
          rules:
            - "f: /tmp/test exists"
          metadata:
            tags:
              - category:
                  - security
                  - compliance
            platforms:
              - os:
                  - linux
                  - windows

      )";

    const auto loader = [yml](const std::string&)
    {
        return LoadFromString(yml);
    };

    const PolicyParser parser("dummy.yaml", loader);
    nlohmann::json j;
    const auto policyOpt = parser.ParsePolicy(j);

    ASSERT_TRUE(policyOpt.has_value());
    EXPECT_EQ(j["checks"][0]["metadata"]["tags"], "category:security, category:compliance");
    EXPECT_EQ(j["checks"][0]["metadata"]["platforms"], "os:linux, os:windows");
}
