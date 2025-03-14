#include "invNormalizer_test.hpp"
#include "inventoryNormalizer.hpp"
#include "test_config.hpp"
#include "test_input.hpp"
#include <cstdio>
#include <fstream>

void InvNormalizerTest::SetUp()
{
    std::ofstream testConfigFile {TEST_CONFIG_FILE_NAME};

    if (testConfigFile.is_open())
    {
        testConfigFile << TEST_CONFIG_FILE_CONTENT;
    }
};

void InvNormalizerTest::TearDown()
{
    std::remove(TEST_CONFIG_FILE_NAME);
};

using ::testing::Return;

TEST_F(InvNormalizerTest, ctor)
{
    EXPECT_NO_THROW((InvNormalizer {TEST_CONFIG_FILE_NAME, "macos"}));
}

TEST_F(InvNormalizerTest, ctorNonExistingFile)
{
    EXPECT_NO_THROW((InvNormalizer {"TEST_CONFIG_FILE_NAME", "macos"}));
}

TEST_F(InvNormalizerTest, ctorWrongFormatConfig)
{
    constexpr auto WRONG_FORMAT_FILE {"wrong_format.json"};
    std::ofstream testConfigFile {WRONG_FORMAT_FILE};

    if (testConfigFile.is_open())
    {
        testConfigFile << R"({"exclusions":[})";
    }

    EXPECT_NO_THROW((InvNormalizer {WRONG_FORMAT_FILE, "macos"}));
    std::remove(WRONG_FORMAT_FILE);
}

TEST_F(InvNormalizerTest, excludeSiriAndiTunes)
{
    auto inputJson(nlohmann::json::parse(TEST_INPUT_DATA));
    const auto size {inputJson.size()};
    const InvNormalizer normalizer {TEST_CONFIG_FILE_NAME, "macos"};
    normalizer.RemoveExcluded("packages", inputJson);
    EXPECT_EQ(size, inputJson.size() + 2);
}

TEST_F(InvNormalizerTest, excludeSingleItemNoMatch)
{
    const auto& origJson {nlohmann::json::parse(R"(
        {
            "description": "com.apple.FaceTime",
            "group": "public.app-category.social-networking",
            "name": "FaceTime",
            "version": "3.0"
        })")};
    nlohmann::json normalized(origJson);
    const InvNormalizer normalizer {TEST_CONFIG_FILE_NAME, "macos"};
    normalizer.RemoveExcluded("packages", normalized);
    EXPECT_EQ(normalized, origJson);
}

TEST_F(InvNormalizerTest, excludeSingleItemMatch)
{
    auto inputJson(nlohmann::json::parse(R"(
        {
            "description": "com.apple.siri.launcher",
            "group": "public.app-category.utilities",
            "name": "Siri",
            "version": "1.0"
        })"));
    const InvNormalizer normalizer {TEST_CONFIG_FILE_NAME, "macos"};
    normalizer.RemoveExcluded("packages", inputJson);
    EXPECT_TRUE(inputJson.empty());
}

TEST_F(InvNormalizerTest, normalizeSingleMicosoft)
{
    auto inputJson(nlohmann::json::parse(R"(
        {
            "description": "com.microsoft.antivirus",
            "group": "public.app-category.security",
            "name": "Microsoft Defender",
            "version": "1.0"
        })"));
    const InvNormalizer normalizer {TEST_CONFIG_FILE_NAME, "macos"};
    normalizer.Normalize("packages", inputJson);
    EXPECT_FALSE(inputJson.empty());
    EXPECT_EQ(inputJson["vendor"], "Microsoft");
}

TEST_F(InvNormalizerTest, normalizeSingleMcAfee1)
{
    auto inputJson(nlohmann::json::parse(R"(
        {
            "description": "com.mcafee.antivirus",
            "group": "public.app-category.security",
            "name": "McAfee Antivirus For Mac",
            "version": "1.0"
        })"));
    const InvNormalizer normalizer {TEST_CONFIG_FILE_NAME, "macos"};
    normalizer.Normalize("packages", inputJson);
    EXPECT_FALSE(inputJson.empty());
    EXPECT_EQ(inputJson["vendor"], "McAfee");
    EXPECT_EQ(inputJson["name"], "Antivirus");
}

TEST_F(InvNormalizerTest, normalizeSingleMcAfee2)
{
    auto inputJson(nlohmann::json::parse(R"(
        {
            "description": "com.mcafee.antivirus",
            "group": "public.app-category.security",
            "name": "McAfee Endpoint Protection For Mac",
            "version": "1.0"
        })"));
    const InvNormalizer normalizer {TEST_CONFIG_FILE_NAME, "macos"};
    normalizer.Normalize("packages", inputJson);
    EXPECT_FALSE(inputJson.empty());
    EXPECT_EQ(inputJson["vendor"], "McAfee");
    EXPECT_EQ(inputJson["name"], "Endpoint Protection");
}

TEST_F(InvNormalizerTest, normalizeSingleTotalDefense1)
{
    auto inputJson(nlohmann::json::parse(R"(
        {
            "description": "com.totaldefense.antivirus",
            "group": "public.app-category.security",
            "name": "TotalDefenseAntivirusforMac",
            "version": "1.0"
        })"));
    const InvNormalizer normalizer {TEST_CONFIG_FILE_NAME, "macos"};
    normalizer.Normalize("packages", inputJson);
    EXPECT_FALSE(inputJson.empty());
    EXPECT_EQ(inputJson["vendor"], "TotalDefense");
    EXPECT_EQ(inputJson["name"], "Anti-Virus");
}

TEST_F(InvNormalizerTest, normalizeSingleTotalDefense2)
{
    auto inputJson(nlohmann::json::parse(R"(
        {
            "description": "com.totaldefense.antivirus",
            "group": "public.app-category.security",
            "name": "TotalDefenseOtherProductforMac",
            "version": "1.0"
        })"));
    const InvNormalizer normalizer {TEST_CONFIG_FILE_NAME, "macos"};
    normalizer.Normalize("packages", inputJson);
    EXPECT_FALSE(inputJson.empty());
    EXPECT_EQ(inputJson["vendor"], "TotalDefense");
    EXPECT_EQ(inputJson["name"], "OtherProduct");
}

TEST_F(InvNormalizerTest, normalizeSingleAVG1)
{
    auto inputJson(nlohmann::json::parse(R"(
        {
            "description": "com.avg.antivirus",
            "group": "public.app-category.security",
            "name": "AVGAntivirus",
            "version": "1.0"
        })"));
    const InvNormalizer normalizer {TEST_CONFIG_FILE_NAME, "macos"};
    normalizer.Normalize("packages", inputJson);
    EXPECT_FALSE(inputJson.empty());
    EXPECT_EQ(inputJson["vendor"], "AVG");
    EXPECT_EQ(inputJson["name"], "Anti-Virus");
}

TEST_F(InvNormalizerTest, normalizeSingleAVG2)
{
    auto inputJson(nlohmann::json::parse(R"(
        {
            "description": "com.avg.antivirus",
            "group": "public.app-category.security",
            "name": "AVGOtherProduct",
            "version": "1.0"
        })"));
    const InvNormalizer normalizer {TEST_CONFIG_FILE_NAME, "macos"};
    normalizer.Normalize("packages", inputJson);
    EXPECT_FALSE(inputJson.empty());
    EXPECT_EQ(inputJson["vendor"], "AVG");
    EXPECT_EQ(inputJson["name"], "OtherProduct");
}

TEST_F(InvNormalizerTest, normalizeSingleKaspersky1)
{
    auto inputJson(nlohmann::json::parse(R"(
        {
            "description": "com.kaspersky.antivirus",
            "group": "public.app-category.security",
            "name": "Kaspersky Antivirus For Mac",
            "version": "1.0"
        })"));
    const InvNormalizer normalizer {TEST_CONFIG_FILE_NAME, "macos"};
    normalizer.Normalize("packages", inputJson);
    EXPECT_FALSE(inputJson.empty());
    EXPECT_EQ(inputJson["name"], "Kaspersky Antivirus");
}

TEST_F(InvNormalizerTest, normalizeSingleKaspersky2)
{
    auto inputJson(nlohmann::json::parse(R"(
        {
            "description": "com.kaspersky.internetsecurity",
            "group": "public.app-category.security",
            "name": "Kaspersky Internet Security For Mac",
            "version": "1.0"
        })"));
    const InvNormalizer normalizer {TEST_CONFIG_FILE_NAME, "macos"};
    normalizer.Normalize("packages", inputJson);
    EXPECT_FALSE(inputJson.empty());
    EXPECT_EQ(inputJson["name"], "Kaspersky Internet Security");
}

TEST_F(InvNormalizerTest, normalizeItemMatch)
{
    auto inputJson(nlohmann::json::parse(TEST_INPUT_DATA));
    const auto origJson(inputJson);
    const InvNormalizer normalizer {TEST_CONFIG_FILE_NAME, "macos"};
    normalizer.Normalize("packages", inputJson);
    EXPECT_EQ(inputJson.size(), origJson.size());
    EXPECT_NE(inputJson, origJson);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
