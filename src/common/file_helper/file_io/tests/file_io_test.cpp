#include "gmock/gmock.h"
#include <gtest/gtest.h>

#include "file_io.hpp"

using namespace testing;

class MockFileIOLocal : public file_io::FileIO
{
public:
    MockFileIOLocal() = default;
    MOCK_METHOD(std::ifstream,
                create_ifstream,
                (const std::string& filePath, std::ios_base::openmode mode),
                (const, override));
    MOCK_METHOD(std::streambuf*, get_rdbuf, (const std::ifstream& file), (const, override));
    MOCK_METHOD(bool, is_open, (const std::ifstream& file), (const, override));
    MOCK_METHOD(bool, get_line, (std::istream & file, std::string& line), (const, override));
};

TEST(FileIOTest, ReadLineByLine_CallsCallbackForEachLine)

{
    auto mockFileIO = std::make_unique<MockFileIOLocal>();
    std::string const fakePath = "fakepath.txt";
    std::ifstream fakeStream;

    EXPECT_CALL(*mockFileIO, create_ifstream(fakePath, std::ios_base::in))
        .WillOnce(testing::Return(ByMove(std::move(fakeStream))));
    EXPECT_CALL(*mockFileIO, is_open(testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mockFileIO, get_line(testing::_, testing::_))
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
    auto mockFileIO = std::make_unique<MockFileIOLocal>();
    std::string const filePath = "fakepath.txt";
    std::ifstream fakeStream;

    EXPECT_CALL(*mockFileIO, create_ifstream(filePath, std::ios_base::in))
        .WillOnce(Return(ByMove(std::move(fakeStream))));
    EXPECT_CALL(*mockFileIO, is_open(_)).WillOnce(Return(false));

    EXPECT_THROW(
        { mockFileIO->readLineByLine(filePath, [](const std::string& /*line*/) { return true; }); },
        std::runtime_error);
}

TEST(FileIOTest, ReadLineByLine_EmptyFile_NoCallbackCalled)
{
    auto mockFileIO = std::make_unique<MockFileIOLocal>();
    std::string const filePath = "fakepath.txt";
    std::ifstream fakeStream;

    EXPECT_CALL(*mockFileIO, create_ifstream(filePath, std::ios_base::in))
        .WillOnce(Return(ByMove(std::move(fakeStream))));
    EXPECT_CALL(*mockFileIO, is_open(_)).WillOnce(Return(true));
    EXPECT_CALL(*mockFileIO, get_line(_, _)).WillOnce(Return(false));

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
    auto mockFileIO = std::make_unique<MockFileIOLocal>();
    std::string const filePath = "fakepath.txt";
    std::ifstream fakeStream("fakepath.txt");
    EXPECT_CALL(*mockFileIO, create_ifstream(filePath, std::ios_base::in))
        .WillOnce(Return(ByMove(std::move(fakeStream))));
    EXPECT_CALL(*mockFileIO, is_open(testing::_)).WillOnce(Return(false));
    std::string const content = mockFileIO->getFileContent(filePath);
    EXPECT_EQ(content, "");
}

TEST(FileIOTest, GetFileContent_FileIsOpen_ReturnsContent)
{
    auto mockFileIO = std::make_unique<MockFileIOLocal>();
    std::string const filePath = "fakepath.txt";
    std::string const fakeData = "fakepath content";

    std::ifstream fakeStream;
    EXPECT_CALL(*mockFileIO, create_ifstream(filePath, testing::_)).WillOnce(Return(ByMove(std::move(fakeStream))));
    EXPECT_CALL(*mockFileIO, is_open(testing::_)).WillOnce(Return(true));

    std::istringstream const iss(fakeData);
    std::streambuf* fakeBuf = iss.rdbuf();
    EXPECT_CALL(*mockFileIO, get_rdbuf(testing::_)).WillOnce(Return(fakeBuf));

    std::string const content = mockFileIO->getFileContent(filePath);
    EXPECT_EQ(content, fakeData);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
