#include "gmock/gmock.h"
#include <gtest/gtest.h>

#include "file_io_utils.hpp"
#include "mocks/mock_file_io_wrapper.hpp"

using namespace testing;

TEST(FileIOTest, ReadLineByLine_CallsCallbackForEachLine)

{
    auto mockFileIOWrapper = std::make_shared<MockFileIOWrapper>();
    auto mockFileIO = std::make_unique<file_io::FileIOUtils>(mockFileIOWrapper);
    const std::string filePath = "fakepath.txt";
    std::unique_ptr<std::ifstream> fakeStream = std::make_unique<std::ifstream>();

    EXPECT_CALL(*mockFileIOWrapper, create_ifstream(filePath, std::ios_base::in))
        .WillOnce(testing::Return(ByMove(std::move(fakeStream))));
    EXPECT_CALL(*mockFileIOWrapper, is_open(testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mockFileIOWrapper, get_line(testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgReferee<1>("line1"), testing::Return(true)))
        .WillOnce(testing::DoAll(testing::SetArgReferee<1>("line2"), testing::Return(true)))
        .WillOnce(testing::Return(false));

    std::vector<std::string> collected;
    mockFileIO->readLineByLine(filePath,
                               [&](const std::string& line)
                               {
                                   collected.push_back(line);
                                   return true;
                               });

    EXPECT_EQ(collected, (std::vector<std::string> {"line1", "line2"}));
}

TEST(FileIOTest, ReadLineByLine_CreateIfstreamFails_ThrowsException)
{
    auto mockFileIOWrapper = std::make_shared<MockFileIOWrapper>();
    auto mockFileIO = std::make_unique<file_io::FileIOUtils>(mockFileIOWrapper);
    const std::string filePath = "fakepath.txt";
    const std::unique_ptr<std::ifstream> fakeStream = std::make_unique<std::ifstream>();

    EXPECT_CALL(*mockFileIOWrapper, create_ifstream(filePath, std::ios_base::in)).WillOnce(Return(nullptr));

    EXPECT_THROW(
        { mockFileIO->readLineByLine(filePath, [](const std::string& /*line*/) { return true; }); },
        std::runtime_error);
}

TEST(FileIOTest, ReadLineByLine_FileNotOpen_ThrowsException)
{
    auto mockFileIOWrapper = std::make_shared<MockFileIOWrapper>();
    auto mockFileIO = std::make_unique<file_io::FileIOUtils>(mockFileIOWrapper);
    const std::string filePath = "fakepath.txt";
    std::unique_ptr<std::ifstream> fakeStream = std::make_unique<std::ifstream>();

    EXPECT_CALL(*mockFileIOWrapper, create_ifstream(filePath, std::ios_base::in))
        .WillOnce(Return(ByMove(std::move(fakeStream))));
    EXPECT_CALL(*mockFileIOWrapper, is_open(_)).WillOnce(Return(false));

    EXPECT_THROW(
        { mockFileIO->readLineByLine(filePath, [](const std::string& /*line*/) { return true; }); },
        std::runtime_error);
}

TEST(FileIOTest, ReadLineByLine_EmptyFile_NoCallbackCalled)
{
    auto mockFileIOWrapper = std::make_shared<MockFileIOWrapper>();
    auto mockFileIO = std::make_unique<file_io::FileIOUtils>(mockFileIOWrapper);
    const std::string filePath = "fakepath.txt";
    std::unique_ptr<std::ifstream> fakeStream = std::make_unique<std::ifstream>();

    EXPECT_CALL(*mockFileIOWrapper, create_ifstream(filePath, std::ios_base::in))
        .WillOnce(Return(ByMove(std::move(fakeStream))));
    EXPECT_CALL(*mockFileIOWrapper, is_open(_)).WillOnce(Return(true));
    EXPECT_CALL(*mockFileIOWrapper, get_line(_, _)).WillOnce(Return(false));

    bool callbackCalled = false;
    mockFileIO->readLineByLine(filePath,
                               [&](const std::string& /*line*/)
                               {
                                   callbackCalled = true;
                                   return true;
                               });

    EXPECT_FALSE(callbackCalled);
}

TEST(FileIOTest, GetFileContent_CreateIfstreamFails_ReturnsEmptyString)
{
    auto mockFileIOWrapper = std::make_shared<MockFileIOWrapper>();
    auto mockFileIO = std::make_unique<file_io::FileIOUtils>(mockFileIOWrapper);
    const std::string filePath = "fakepath.txt";
    const std::unique_ptr<std::ifstream> fakeStream = std::make_unique<std::ifstream>();

    EXPECT_CALL(*mockFileIOWrapper, create_ifstream(filePath, std::ios_base::in)).WillOnce(Return(nullptr));

    const std::string content = mockFileIO->getFileContent(filePath);
    EXPECT_EQ(content, "");
}

TEST(FileIOTest, GetFileContent_FileNotOpen_ReturnsEmptyString)
{
    auto mockFileIOWrapper = std::make_shared<MockFileIOWrapper>();
    auto mockFileIO = std::make_unique<file_io::FileIOUtils>(mockFileIOWrapper);
    const std::string filePath = "fakepath.txt";
    std::unique_ptr<std::ifstream> fakeStream = std::make_unique<std::ifstream>();

    EXPECT_CALL(*mockFileIOWrapper, create_ifstream(filePath, std::ios_base::in))
        .WillOnce(Return(ByMove(std::move(fakeStream))));
    EXPECT_CALL(*mockFileIOWrapper, is_open(testing::_)).WillOnce(Return(false));
    const std::string content = mockFileIO->getFileContent(filePath);
    EXPECT_EQ(content, "");
}

TEST(FileIOTest, GetFileContent_FileIsOpen_ReturnsContent)
{
    auto mockFileIOWrapper = std::make_shared<MockFileIOWrapper>();
    auto mockFileIO = std::make_unique<file_io::FileIOUtils>(mockFileIOWrapper);
    const std::string filePath = "fakepath.txt";
    const std::string fakeData = "fakepath content";

    std::unique_ptr<std::ifstream> fakeStream = std::make_unique<std::ifstream>();

    EXPECT_CALL(*mockFileIOWrapper, create_ifstream(filePath, testing::_))
        .WillOnce(Return(ByMove(std::move(fakeStream))));
    EXPECT_CALL(*mockFileIOWrapper, is_open(testing::_)).WillOnce(Return(true));

    const std::istringstream iss(fakeData);
    std::streambuf* fakeBuf = iss.rdbuf();
    EXPECT_CALL(*mockFileIOWrapper, get_rdbuf(testing::_)).WillOnce(Return(fakeBuf));

    const std::string content = mockFileIO->getFileContent(filePath);
    EXPECT_EQ(content, fakeData);
}

TEST(FileIOTest, GetBinaryContent_FileIsNotOpen_ReturnsEmptyVector)
{
    auto mockFileIOWrapper = std::make_shared<MockFileIOWrapper>();
    auto mockFileIO = std::make_unique<file_io::FileIOUtils>(mockFileIOWrapper);
    const std::string filePath = "fakepath.txt";
    std::unique_ptr<std::ifstream> fakeStream = std::make_unique<std::ifstream>();

    EXPECT_CALL(*mockFileIOWrapper, create_ifstream(filePath, testing::_))
        .WillOnce(Return(ByMove(std::move(fakeStream))));
    EXPECT_CALL(*mockFileIOWrapper, is_open(testing::_)).WillOnce(Return(false));

    const std::vector<char> content = mockFileIO->getBinaryContent(filePath);
    EXPECT_EQ(content.size(), 0);
}

TEST(FileIOTest, GetBinaryContent_CreateIfstreamFails_ReturnsEmptyVector)
{
    auto mockFileIOWrapper = std::make_shared<MockFileIOWrapper>();
    auto mockFileIO = std::make_unique<file_io::FileIOUtils>(mockFileIOWrapper);
    const std::string filePath = "fakepath.txt";
    const std::unique_ptr<std::ifstream> fakeStream = std::make_unique<std::ifstream>();

    EXPECT_CALL(*mockFileIOWrapper, create_ifstream(filePath, testing::_)).WillOnce(Return(nullptr));

    const std::vector<char> content = mockFileIO->getBinaryContent(filePath);
    EXPECT_EQ(content.size(), 0);
}

TEST(FileIOTest, GetBinaryContent_FileIsOpen_BufferIsNull_ReturnsEmptyVector)
{
    auto mockFileIOWrapper = std::make_shared<MockFileIOWrapper>();
    auto mockFileIO = std::make_unique<file_io::FileIOUtils>(mockFileIOWrapper);
    const std::string filePath = "fakepath.txt";
    std::unique_ptr<std::ifstream> fakeStream = std::make_unique<std::ifstream>();

    EXPECT_CALL(*mockFileIOWrapper, create_ifstream(filePath, testing::_))
        .WillOnce(Return(ByMove(std::move(fakeStream))));
    EXPECT_CALL(*mockFileIOWrapper, is_open(testing::_)).WillOnce(Return(true));
    EXPECT_CALL(*mockFileIOWrapper, get_rdbuf(testing::_)).WillOnce(Return(nullptr));

    const std::vector<char> content = mockFileIO->getBinaryContent(filePath);
    EXPECT_EQ(content.size(), 0);
}

TEST(FileIOTest, GetBinaryContent_FileIsOpen_BufferIsNotNull_ReturnsContent)
{
    auto mockFileIOWrapper = std::make_shared<MockFileIOWrapper>();
    auto mockFileIO = std::make_unique<file_io::FileIOUtils>(mockFileIOWrapper);
    const std::string filePath = "fakepath.txt";
    const std::string fakeData = "fakepath content";
    std::unique_ptr<std::ifstream> fakeStream = std::make_unique<std::ifstream>();

    const std::istringstream iss(fakeData);
    std::streambuf* fakeBuf = iss.rdbuf();

    EXPECT_CALL(*mockFileIOWrapper, create_ifstream(filePath, testing::_))
        .WillOnce(Return(ByMove(std::move(fakeStream))));
    EXPECT_CALL(*mockFileIOWrapper, is_open(testing::_)).WillOnce(Return(true));
    EXPECT_CALL(*mockFileIOWrapper, get_rdbuf(testing::_)).WillOnce(Return(fakeBuf));

    const std::vector<char> content = mockFileIO->getBinaryContent(filePath);
    EXPECT_NE(content.size(), 0);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
