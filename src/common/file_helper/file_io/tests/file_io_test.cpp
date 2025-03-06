#include "gmock/gmock.h"
#include <gtest/gtest.h>

#include "file_io_utils.hpp"
#include "file_io_wrapper.hpp"
#include "mocks/mock_file_io_wrapper.hpp"

using namespace testing;

TEST(FileIOTest, ReadLineByLine_CallsCallbackForEachLine)

{
    auto mockFileIOWrapper = std::make_shared<MockFileIOWrapper>();
    auto mockFileIO = std::make_unique<file_io::FileIOUtils>(mockFileIOWrapper);
    std::string const fakePath = "fakepath.txt";
    std::ifstream fakeStream;

    EXPECT_CALL(*mockFileIOWrapper, create_ifstream(fakePath, std::ios_base::in))
        .WillOnce(testing::Return(ByMove(std::move(fakeStream))));
    EXPECT_CALL(*mockFileIOWrapper, is_open(testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mockFileIOWrapper, get_line(testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgReferee<1>("line1"), testing::Return(true)))
        .WillOnce(testing::DoAll(testing::SetArgReferee<1>("line2"), testing::Return(true)))
        .WillOnce(testing::Return(false));

    std::vector<std::string> collected;
    mockFileIO->readLineByLine(fakePath,
                               [&](const std::string& line)
                               {
                                   collected.push_back(line);
                                   return true;
                               });

    EXPECT_EQ(collected, (std::vector<std::string> {"line1", "line2"}));
}

TEST(FileIOTest, ReadLineByLine_FileNotOpen_ThrowsException)
{
    auto mockFileIOWrapper = std::make_shared<MockFileIOWrapper>();
    auto mockFileIO = std::make_unique<file_io::FileIOUtils>(mockFileIOWrapper);
    std::string const filePath = "fakepath.txt";
    std::ifstream fakeStream;

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
    std::string const filePath = "fakepath.txt";
    std::ifstream fakeStream;

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

TEST(FileIOTest, GetFileContent_FileNotOpen_ReturnsEmptyString)
{
    auto mockFileIOWrapper = std::make_shared<MockFileIOWrapper>();
    auto mockFileIO = std::make_unique<file_io::FileIOUtils>(mockFileIOWrapper);
    std::string const filePath = "fakepath.txt";
    std::ifstream fakeStream("fakepath.txt");
    EXPECT_CALL(*mockFileIOWrapper, create_ifstream(filePath, std::ios_base::in))
        .WillOnce(Return(ByMove(std::move(fakeStream))));
    EXPECT_CALL(*mockFileIOWrapper, is_open(testing::_)).WillOnce(Return(false));
    std::string const content = mockFileIO->getFileContent(filePath);
    EXPECT_EQ(content, "");
}

TEST(FileIOTest, GetFileContent_FileIsOpen_ReturnsContent)
{
    auto mockFileIOWrapper = std::make_shared<MockFileIOWrapper>();
    auto mockFileIO = std::make_unique<file_io::FileIOUtils>(mockFileIOWrapper);
    std::string const filePath = "fakepath.txt";
    std::string const fakeData = "fakepath content";

    std::ifstream fakeStream;
    EXPECT_CALL(*mockFileIOWrapper, create_ifstream(filePath, testing::_)).WillOnce(Return(ByMove(std::move(fakeStream))));
    EXPECT_CALL(*mockFileIOWrapper, is_open(testing::_)).WillOnce(Return(true));

    std::istringstream const iss(fakeData);
    std::streambuf* fakeBuf = iss.rdbuf();
    EXPECT_CALL(*mockFileIOWrapper, get_rdbuf(testing::_)).WillOnce(Return(fakeBuf));

    std::string const content = mockFileIO->getFileContent(filePath);
    EXPECT_EQ(content, fakeData);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
