#include <gtest/gtest.h>

#include <filesystem_wrapper.hpp>
#include <fstream>
#include <thread>
#include <unordered_map>

constexpr auto FS_MS_WAIT_TIME {50ull};

constexpr auto ITERATION_LIMIT {10u};

#ifndef _WIN32
constexpr auto EXPAND_PATH_1 {"/tmp/wazuh_test/prefix_1_data/prefix1_1"};
constexpr auto EXPAND_PATH_2 {"/tmp/wazuh_test/prefix_1_data/prefix1_2"};
constexpr auto EXPAND_PATH_3 {"/tmp/wazuh_test/prefix_2_data/prefix2_1"};
constexpr auto EXPAND_PATH_4 {"/tmp/wazuh_test/prefix_2_data/prefix2_2"};
constexpr auto EXPAND_PATH_5 {"/tmp/wazuh_test/dummy"};
constexpr auto EXPAND_PATH_6 {"/tmp/wazuh_test/dummy.txt"};
constexpr auto PATH_TO_EXPAND_1 {"/tmp/wazuh_test/dum*"};
constexpr auto PATH_TO_EXPAND_2 {"/tmp/wazuh_test/prefix_*_data/*"};
constexpr auto PATH_TO_EXPAND_3 {"/tmp/wazuh_test/prefix_*_data/prefix*"};
constexpr auto PATH_TO_EXPAND_4 {"/tmp/wazuh_test/prefix_*_data/*_1"};
constexpr auto PATH_TO_EXPAND_5 {"/tmp/wazuh_test/prefix_?_data/*_1"};
constexpr auto PATH_TO_EXPAND_6 {"/tmp/wazuh_test/prefix_*_data/prefix?*1"};
constexpr auto PATH_TO_EXPAND_7 {"/tmp/wazuh_test/*/*1"};
constexpr auto TMP_PATH {"/tmp"};
constexpr auto ROOT_PATH {"/tmp/wazuh_test"};
constexpr auto ROOT_PATH_1 {"/tmp/wazuh_test/prefix_1_data"};
constexpr auto ROOT_PATH_2 {"/tmp/wazuh_test/prefix_2_data"};
constexpr auto ROOT_PATH_DUMMY {"/tmp/wazuh_test/dummy"};
constexpr auto DUMMY_FILE {"/tmp/wazuh_test/dummy.txt"};
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
constexpr auto PATH_TO_EXPAND_7 {"C:\\tmp\\wazuh_test\\*\\*1"};
constexpr auto TMP_PATH {"C:\\tmp"};
constexpr auto ROOT_PATH {"C:\\tmp\\wazuh_test"};
constexpr auto ROOT_PATH_1 {"C:\\tmp\\wazuh_test\\prefix_1_data"};
constexpr auto ROOT_PATH_2 {"C:\\tmp\\wazuh_test\\prefix_2_data"};
constexpr auto ROOT_PATH_DUMMY {"C:\\tmp\\wazuh_test\\dummy"};
constexpr auto DUMMY_FILE {"C:\\tmp\\wazuh_test\\dummy.txt"};
#endif

class FileSystemTest : public ::testing::Test
{
protected:
    static void TearDownTestSuite()
    {
        const auto fsWrapper = std::make_unique<filesystem_wrapper::FileSystemWrapper>();
        fsWrapper->remove_all(ROOT_PATH);
        auto iteration {0u};

        while (fsWrapper->exists(ROOT_PATH))
        {
            if (iteration++ > ITERATION_LIMIT)
            {
                FAIL() << "Unable to remove " << ROOT_PATH;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(FS_MS_WAIT_TIME));
        }
    }

    static void SetUpTestSuite()
    {
        const auto fsWrapper = std::make_unique<filesystem_wrapper::FileSystemWrapper>();

        auto iteration {0u};
        fsWrapper->remove_all(ROOT_PATH);

        while (fsWrapper->exists(ROOT_PATH))
        {
            if (iteration++ > ITERATION_LIMIT)
            {
                FAIL() << "Unable to remove " << ROOT_PATH;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(FS_MS_WAIT_TIME));
        }

        fsWrapper->create_directories(TMP_PATH);

        iteration = 0u;

        while (!fsWrapper->exists(TMP_PATH))
        {
            if (iteration++ > ITERATION_LIMIT)
            {
                FAIL() << "Unable to create " << TMP_PATH;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(FS_MS_WAIT_TIME));
        }

        fsWrapper->create_directories(ROOT_PATH);

        iteration = 0u;

        while (!fsWrapper->exists(ROOT_PATH))
        {
            if (iteration++ > ITERATION_LIMIT)
            {
                FAIL() << "Unable to create " << ROOT_PATH;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(FS_MS_WAIT_TIME));
        }

        fsWrapper->create_directories(ROOT_PATH_1);
        fsWrapper->create_directories(ROOT_PATH_2);
        fsWrapper->create_directories(ROOT_PATH_DUMMY);

        iteration = 0u;

        while (!fsWrapper->exists(ROOT_PATH_1) || !fsWrapper->exists(ROOT_PATH_2) ||
               !fsWrapper->exists(ROOT_PATH_DUMMY))
        {
            if (iteration++ > ITERATION_LIMIT)
            {
                FAIL() << "Unable to create " << ROOT_PATH_1 << " or " << ROOT_PATH_2 << " or " << ROOT_PATH_DUMMY;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(FS_MS_WAIT_TIME));
        }

        // Create dummy file
        std::ofstream dummyFile(DUMMY_FILE);
        dummyFile << "dummy";
        dummyFile.close();

        fsWrapper->create_directories(EXPAND_PATH_1);
        fsWrapper->create_directories(EXPAND_PATH_2);
        fsWrapper->create_directories(EXPAND_PATH_3);
        fsWrapper->create_directories(EXPAND_PATH_4);

        iteration = 0u;

        while (!fsWrapper->exists(EXPAND_PATH_1) || !fsWrapper->exists(EXPAND_PATH_2) ||
               !fsWrapper->exists(EXPAND_PATH_3) || !fsWrapper->exists(EXPAND_PATH_4))
        {
            if (iteration++ > ITERATION_LIMIT)
            {
                FAIL() << "Unable to create " << EXPAND_PATH_1 << " or " << EXPAND_PATH_2 << " or " << EXPAND_PATH_3
                       << " or " << EXPAND_PATH_4;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(FS_MS_WAIT_TIME));
        }
    }
};

TEST_F(FileSystemTest, FilesystemExpandSimpleWildcard)
{
    constexpr auto PATH_MATCH_SIZE {2u};
    std::deque<std::string> output;
    std::unordered_map<std::string, uint32_t> outputMap;

    const auto fsWrapper = std::make_unique<filesystem_wrapper::FileSystemWrapper>();
    fsWrapper->expand_absolute_path(PATH_TO_EXPAND_1, output);

    for (const auto& item : output)
    {
        outputMap[item]++;
    }

    EXPECT_TRUE(outputMap.at(EXPAND_PATH_5) == 1);
    EXPECT_TRUE(outputMap.at(EXPAND_PATH_6) == 1);
    EXPECT_EQ(output.size(), PATH_MATCH_SIZE);
}

TEST_F(FileSystemTest, FilesystemExpandWildcard)
{
    constexpr auto PATH_MATCH_SIZE {4u};
    std::deque<std::string> output;
    std::unordered_map<std::string, uint32_t> outputMap;

    const auto fsWrapper = std::make_unique<filesystem_wrapper::FileSystemWrapper>();
    fsWrapper->expand_absolute_path(PATH_TO_EXPAND_2, output);

    for (const auto& item : output)
    {
        outputMap[item]++;
    }

    EXPECT_TRUE(outputMap.at(EXPAND_PATH_1) == 1);
    EXPECT_TRUE(outputMap.at(EXPAND_PATH_2) == 1);
    EXPECT_TRUE(outputMap.at(EXPAND_PATH_3) == 1);
    EXPECT_TRUE(outputMap.at(EXPAND_PATH_4) == 1);
    EXPECT_EQ(output.size(), PATH_MATCH_SIZE);
}

TEST_F(FileSystemTest, FilesystemExpandWildcardWithPrefix)
{
    constexpr auto PATH_MATCH_SIZE {4u};
    std::deque<std::string> output;
    std::unordered_map<std::string, uint32_t> outputMap;

    const auto fsWrapper = std::make_unique<filesystem_wrapper::FileSystemWrapper>();
    fsWrapper->expand_absolute_path(PATH_TO_EXPAND_3, output);

    for (const auto& item : output)
    {
        outputMap[item]++;
    }

    EXPECT_TRUE(outputMap.at(EXPAND_PATH_1) == 1);
    EXPECT_TRUE(outputMap.at(EXPAND_PATH_2) == 1);
    EXPECT_TRUE(outputMap.at(EXPAND_PATH_3) == 1);
    EXPECT_TRUE(outputMap.at(EXPAND_PATH_4) == 1);
    EXPECT_EQ(output.size(), PATH_MATCH_SIZE);
}

TEST_F(FileSystemTest, FilesystemExpandWildcardWithPrefixFull)
{
    constexpr auto PATH_MATCH_SIZE {2u};
    std::deque<std::string> output;
    std::unordered_map<std::string, uint32_t> outputMap;

    const auto fsWrapper = std::make_unique<filesystem_wrapper::FileSystemWrapper>();
    fsWrapper->expand_absolute_path(PATH_TO_EXPAND_7, output);

    for (const auto& item : output)
    {
        outputMap[item]++;
    }

    EXPECT_TRUE(outputMap.at(EXPAND_PATH_1) == 1);
    EXPECT_TRUE(outputMap.at(EXPAND_PATH_3) == 1);
    EXPECT_EQ(output.size(), PATH_MATCH_SIZE);
}

TEST_F(FileSystemTest, FilesystemExpandWildcardWithSuffix)
{
    constexpr auto PATH_MATCH_SIZE {2u};
    std::deque<std::string> output;
    std::unordered_map<std::string, uint32_t> outputMap;

    const auto fsWrapper = std::make_unique<filesystem_wrapper::FileSystemWrapper>();
    fsWrapper->expand_absolute_path(PATH_TO_EXPAND_4, output);

    for (const auto& item : output)
    {
        outputMap[item]++;
    }

    EXPECT_TRUE(outputMap.at(EXPAND_PATH_1) == 1);
    EXPECT_TRUE(outputMap.at(EXPAND_PATH_3) == 1);

    EXPECT_EQ(output.size(), PATH_MATCH_SIZE);
}

TEST_F(FileSystemTest, FilesystemExpandWildcardWithQuestionMark)
{
    constexpr auto PATH_MATCH_SIZE {2u};
    std::deque<std::string> output;
    std::unordered_map<std::string, uint32_t> outputMap;

    const auto fsWrapper = std::make_unique<filesystem_wrapper::FileSystemWrapper>();
    fsWrapper->expand_absolute_path(PATH_TO_EXPAND_5, output);

    for (const auto& item : output)
    {
        outputMap[item]++;
    }

    EXPECT_TRUE(outputMap.at(EXPAND_PATH_1) == 1);
    EXPECT_TRUE(outputMap.at(EXPAND_PATH_3) == 1);
    EXPECT_EQ(output.size(), PATH_MATCH_SIZE);
}

TEST_F(FileSystemTest, FilesystemExpandWildcardWithQuestionMark2)
{
    constexpr auto PATH_MATCH_SIZE {2u};
    std::deque<std::string> output;
    std::unordered_map<std::string, uint32_t> outputMap;

    const auto fsWrapper = std::make_unique<filesystem_wrapper::FileSystemWrapper>();
    fsWrapper->expand_absolute_path(PATH_TO_EXPAND_6, output);

    for (const auto& item : output)
    {
        outputMap[item]++;
    }

    EXPECT_TRUE(outputMap.at(EXPAND_PATH_1) == 1);
    EXPECT_TRUE(outputMap.at(EXPAND_PATH_3) == 1);
    EXPECT_EQ(output.size(), PATH_MATCH_SIZE);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
