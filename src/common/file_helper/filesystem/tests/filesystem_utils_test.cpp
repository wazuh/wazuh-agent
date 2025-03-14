#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <filesystem_utils.hpp>
#include "mocks/mock_filesystem_wrapper.hpp"

#include <filesystem>
#include <fstream>
#include <thread>
#include <unordered_map>
#include <deque>
#include <vector>
#include <string>
#include <algorithm>

#ifndef _WIN32
constexpr auto EXPANDED_PATH_1 {"/tmp/wazuh_test/prefix_1_data/prefix1_1"};
constexpr auto EXPANDED_PATH_2 {"/tmp/wazuh_test/prefix_1_data/prefix1_2"};
constexpr auto EXPANDED_PATH_3 {"/tmp/wazuh_test/prefix_2_data/prefix2_1"};
constexpr auto EXPANDED_PATH_4 {"/tmp/wazuh_test/prefix_2_data/prefix2_2"};
constexpr auto EXPANDED_PATH_5 {"/tmp/wazuh_test/dummy"};
constexpr auto EXPANDED_PATH_6 {"/tmp/wazuh_test/dummy.txt"};

constexpr auto PATH_TO_EXPAND_1 {"/tmp/wazuh_test/dum*"};
constexpr auto PATH_TO_EXPAND_2 {"/tmp/wazuh_test/prefix_*_data/*"};
constexpr auto PATH_TO_EXPAND_3 {"/tmp/wazuh_test/prefix_*_data/prefix*"};
constexpr auto PATH_TO_EXPAND_4 {"/tmp/wazuh_test/prefix_*_data/*_1"};
constexpr auto PATH_TO_EXPAND_5 {"/tmp/wazuh_test/prefix_?_data/*_1"};
constexpr auto PATH_TO_EXPAND_6 {"/tmp/wazuh_test/prefix_*_data/prefix?*1"};
#else
constexpr auto EXPAND_PATH_1 {"C:\\tmp\\wazuh_test\\prefix_1_data\\prefix1_1"};
constexpr auto EXPAND_PATH_2 {"C:\\tmp\\wazuh_test\\prefix_1_data\\prefix1_2"};
constexpr auto EXPAND_PATH_3 {"C:\\tmp\\wazuh_test\\prefix_2_data\\prefix2_1"};
constexpr auto EXPAND_PATH_4 {"C:\\tmp\\wazuh_test\\prefix_2_data\\prefix2_2"};
constexpr auto EXPAND_PATH_5 {"C:\\tmp\\wazuh_test\\dummy"};
constexpr auto EXPAND_PATH_6 {"C:\\tmp\\wazuh_test\\dummy.txt"};

constexpr auto PATH_TO_EXPAND_1 {"C:\\tmp\\wazuh_test\\dum*"};
constexpr auto PATH_TO_EXPAND_2 {"C:\\tmp\\wazuh_test\\prefix_*_data\\*"};
constexpr auto PATH_TO_EXPAND_3 {"C:\\tmp\\wazuh_test\\prefix_*_data\\prefix*"};
constexpr auto PATH_TO_EXPAND_4 {"C:\\tmp\\wazuh_test\\prefix_*_data\\*_1"};
constexpr auto PATH_TO_EXPAND_5 {"C:\\tmp\\wazuh_test\\prefix_?_data\\*_1"};
constexpr auto PATH_TO_EXPAND_6 {"C:\\tmp\\wazuh_test\\prefix_*_data\\prefix?*1"};
#endif

struct FsElement
{
    std::string path {};
    bool is_dir {};
};

class FileSystemTest : public ::testing::Test
{
protected:
    std::deque<std::string> output;
    std::unordered_map<std::string, uint32_t> outputMap;
    std::unique_ptr<filesystem::FileSystemUtils> fsUtils;
    std::shared_ptr<MockFileSystemWrapper> mockWrapper;

    std::vector<FsElement> fsElements { {EXPANDED_PATH_1, true}
                                , {EXPANDED_PATH_2, true}
                                , {EXPANDED_PATH_3, true}
                                , {EXPANDED_PATH_4, true}
                                , {EXPANDED_PATH_5, true}
                                , {EXPANDED_PATH_6, false}};

    void TearDown() override
    {
        fsUtils.reset();
        mockWrapper.reset();
    }

    void SetUp() override
    {
        mockWrapper = std::make_shared<MockFileSystemWrapper>();
        fsUtils = std::make_unique<filesystem::FileSystemUtils>(mockWrapper);
    }

    void SetupFilesystemWrapperExpectations()
    {
        EXPECT_CALL(*mockWrapper, exists(::testing::_))
            .WillRepeatedly(::testing::Invoke([&](const std::string& searchPath)->bool
                {
                    return std::any_of(fsElements.begin(), fsElements.end(),
                    [&searchPath](const FsElement& dir) {
                        return dir.path.compare(0, searchPath.length(), searchPath) == 0;
                    });
                }));

        EXPECT_CALL(*mockWrapper, is_directory(::testing::_))
        .WillRepeatedly(::testing::Invoke([&](const std::string& searchPath)->bool
        {
            auto elem = std::find_if(fsElements.begin(), fsElements.end(),
            [&searchPath](const FsElement& dir)
            {
                return dir.path.compare(0, searchPath.length(), searchPath) == 0;
            });

            return elem != fsElements.end() ? elem->is_dir : false;
        }));

        EXPECT_CALL(*mockWrapper, list_directory(::testing::_))
        .WillRepeatedly(::testing::Invoke([&](const std::string& searchPath)->std::vector<std::filesystem::path>
        {
            // get matching elements
            std::vector<FsElement> matches(fsElements.size());
            auto it = std::copy_if(fsElements.begin(), fsElements.end(), matches.begin(),
            [&searchPath](const FsElement& dir)
            {
                return dir.path.compare(0, searchPath.length(), searchPath) == 0;
            });
            matches.resize(static_cast<std::size_t>(std::distance(matches.begin(), it)));

            // convert to paths and remove all extra path components but one past the ones in searchPath
            std::vector<std::filesystem::path> pathVector;
            std::transform(matches.begin(), matches.end(), std::back_inserter(pathVector),
                [&](const FsElement& elem)
                {
                    auto basePath = std::filesystem::path(searchPath);
                    auto curPath = std::filesystem::path(elem.path);

                    std::filesystem::path returnPath = basePath;

                    auto longPathIter = curPath.begin();
                    std::advance(longPathIter, std::distance(basePath.begin(), basePath.end())); // Advance to the first component past the shorter path

                    returnPath /= *longPathIter;

                    return returnPath;
                });

            // remove duplicates
            std::sort(pathVector.begin(), pathVector.end());
            auto end = std::unique(pathVector.begin(), pathVector.end());
            pathVector.erase(end, pathVector.end());

            return pathVector;
        }));

    }
};

TEST_F(FileSystemTest, FilesystemExpandSimpleWildcard)
{
    SetupFilesystemWrapperExpectations();

    constexpr auto PATH_MATCH_SIZE {2u};

    fsUtils->expand_absolute_path(PATH_TO_EXPAND_1, output);

    for (const auto& item : output)
    {
        outputMap[item]++;
    }

    EXPECT_TRUE(outputMap.at(EXPANDED_PATH_5) == 1);
    EXPECT_TRUE(outputMap.at(EXPANDED_PATH_6) == 1);
    EXPECT_EQ(output.size(), PATH_MATCH_SIZE);
}

TEST_F(FileSystemTest, FilesystemExpandWildcard)
{
    SetupFilesystemWrapperExpectations();
    constexpr auto PATH_MATCH_SIZE {4u};

    fsUtils->expand_absolute_path(PATH_TO_EXPAND_2, output);

    for (const auto& item : output)
    {
        outputMap[item]++;
    }

    EXPECT_TRUE(outputMap.at(EXPANDED_PATH_1) == 1);
    EXPECT_TRUE(outputMap.at(EXPANDED_PATH_2) == 1);
    EXPECT_TRUE(outputMap.at(EXPANDED_PATH_3) == 1);
    EXPECT_TRUE(outputMap.at(EXPANDED_PATH_4) == 1);
    EXPECT_EQ(output.size(), PATH_MATCH_SIZE);
}

TEST_F(FileSystemTest, FilesystemExpandWildcardWithPrefix)
{
    SetupFilesystemWrapperExpectations();
    constexpr auto PATH_MATCH_SIZE {4u};

    fsUtils->expand_absolute_path(PATH_TO_EXPAND_3, output);

    for (const auto& item : output)
    {
        outputMap[item]++;
    }

    EXPECT_TRUE(outputMap.at(EXPANDED_PATH_1) == 1);
    EXPECT_TRUE(outputMap.at(EXPANDED_PATH_2) == 1);
    EXPECT_TRUE(outputMap.at(EXPANDED_PATH_3) == 1);
    EXPECT_TRUE(outputMap.at(EXPANDED_PATH_4) == 1);
    EXPECT_EQ(output.size(), PATH_MATCH_SIZE);
}

TEST_F(FileSystemTest, FilesystemExpandWildcardWithSuffix)
{
    SetupFilesystemWrapperExpectations();
    constexpr auto PATH_MATCH_SIZE {2u};

    fsUtils->expand_absolute_path(PATH_TO_EXPAND_4, output);

    for (const auto& item : output)
    {
        outputMap[item]++;
    }

    EXPECT_TRUE(outputMap.at(EXPANDED_PATH_1) == 1);
    EXPECT_TRUE(outputMap.at(EXPANDED_PATH_3) == 1);

    EXPECT_EQ(output.size(), PATH_MATCH_SIZE);
}

TEST_F(FileSystemTest, FilesystemExpandWildcardWithQuestionMark)
{
    SetupFilesystemWrapperExpectations();
    constexpr auto PATH_MATCH_SIZE {2u};

    fsUtils->expand_absolute_path(PATH_TO_EXPAND_5, output);

    for (const auto& item : output)
    {
        outputMap[item]++;
    }

    EXPECT_TRUE(outputMap.at(EXPANDED_PATH_1) == 1);
    EXPECT_TRUE(outputMap.at(EXPANDED_PATH_3) == 1);
    EXPECT_EQ(output.size(), PATH_MATCH_SIZE);
}

TEST_F(FileSystemTest, FilesystemExpandWildcardWithQuestionMark2)
{
    SetupFilesystemWrapperExpectations();
    constexpr auto PATH_MATCH_SIZE {2u};

    fsUtils->expand_absolute_path(PATH_TO_EXPAND_6, output);

    for (const auto& item : output)
    {
        outputMap[item]++;
    }

    EXPECT_TRUE(outputMap.at(EXPANDED_PATH_1) == 1);
    EXPECT_TRUE(outputMap.at(EXPANDED_PATH_3) == 1);
    EXPECT_EQ(output.size(), PATH_MATCH_SIZE);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
