#include "yaml_utils.hpp"

#include <gtest/gtest.h>

#include <yaml-cpp/yaml.h>

/// @brief Test for merging two simple YAML maps with unique keys.
TEST(MergeYamlNodesTest, MergeSimpleMaps)
{
    YAML::Node base = YAML::Load(R"(
                        key1: value1
                        key2: value2
                        )");
    YAML::Node additional = YAML::Load(R"(
                        key3: value3
                        key4: value4
                        )");

    MergeYamlNodes(base, additional);

    EXPECT_EQ(base["key1"].as<std::string>(), "value1");
    EXPECT_EQ(base["key2"].as<std::string>(), "value2");
    EXPECT_EQ(base["key3"].as<std::string>(), "value3");
    EXPECT_EQ(base["key4"].as<std::string>(), "value4");
}

/// @brief Test for overriding a key in the base YAML with a value from additional YAML.
TEST(MergeYamlNodesTest, OverrideScalarValue)
{
    YAML::Node base = YAML::Load(R"(
                        key1: value1
                        key2: value2
                        )");
    YAML::Node additional = YAML::Load(R"(
                        key2: new_value
                        )");

    MergeYamlNodes(base, additional);

    EXPECT_EQ(base["key1"].as<std::string>(), "value1");
    EXPECT_EQ(base["key2"].as<std::string>(), "new_value");
}

/// @brief Test for merging nested YAML maps.
TEST(MergeYamlNodesTest, MergeNestedMaps)
{
    YAML::Node base = YAML::Load(R"(
                        parent:
                            child1: value1
                            child2: value2
                            )");
    YAML::Node additional = YAML::Load(R"(
                        parent:
                            child2: new_value
                            child3: value3
                            )");

    MergeYamlNodes(base, additional);

    ASSERT_TRUE(base["parent"].IsMap());
    EXPECT_EQ(base["parent"]["child1"].as<std::string>(), "value1");
    EXPECT_EQ(base["parent"]["child2"].as<std::string>(), "new_value");
    EXPECT_EQ(base["parent"]["child3"].as<std::string>(), "value3");
}

/// @brief Test for merging YAML sequences while preserving order and avoiding duplicates.
TEST(MergeYamlNodesTest, MergeSequences)
{
    YAML::Node base = YAML::Load(R"(
                        key:
                            - item1
                            - item2
                            )");
    YAML::Node additional = YAML::Load(R"(
                        key:
                            - item2
                            - item3
                            )");

    MergeYamlNodes(base, additional);

    ASSERT_TRUE(base["key"].IsSequence());
    EXPECT_EQ(base["key"].size(), 3);
    EXPECT_EQ(base["key"][0].as<std::string>(), "item1");
    EXPECT_EQ(base["key"][1].as<std::string>(), "item2");
    EXPECT_EQ(base["key"][2].as<std::string>(), "item3");
}

/// @brief Test for adding new keys to a base YAML with existing nested maps.
TEST(MergeYamlNodesTest, AddNewKeysToNestedMap)
{
    YAML::Node base = YAML::Load(R"(
                        parent:
                            child1: value1
                            )");
    YAML::Node additional = YAML::Load(R"(
                        parent:
                            child2: value2
                            )");

    MergeYamlNodes(base, additional);

    ASSERT_TRUE(base["parent"].IsMap());
    EXPECT_EQ(base["parent"]["child1"].as<std::string>(), "value1");
    EXPECT_EQ(base["parent"]["child2"].as<std::string>(), "value2");
}

/// @brief Test for merging when base YAML is empty.
TEST(MergeYamlNodesTest, BaseYamlEmpty)
{
    YAML::Node base = YAML::Load("{}");
    YAML::Node additional = YAML::Load(R"(
                                key1: value1
                                key2: value2
                                )");

    MergeYamlNodes(base, additional);

    EXPECT_EQ(base["key1"].as<std::string>(), "value1");
    EXPECT_EQ(base["key2"].as<std::string>(), "value2");
}

/// @brief Test for merging when additional YAML is empty.
TEST(MergeYamlNodesTest, AdditionalYamlEmpty)
{
    YAML::Node base = YAML::Load(R"(
                        key1: value1
                        key2: value2
                        )");
    YAML::Node additional = YAML::Load("{}");

    MergeYamlNodes(base, additional);

    EXPECT_EQ(base["key1"].as<std::string>(), "value1");
    EXPECT_EQ(base["key2"].as<std::string>(), "value2");
}

/// @brief Test for handling edge cases with scalar values.
TEST(MergeYamlNodesTest, ScalarsOverwriteCorrectly)
{
    YAML::Node base = YAML::Load(R"(
                        key: old_value
                        )");
    YAML::Node additional = YAML::Load(R"(
                        key: new_value
                        )");

    MergeYamlNodes(base, additional);

    EXPECT_EQ(base["key"].as<std::string>(), "new_value");
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
