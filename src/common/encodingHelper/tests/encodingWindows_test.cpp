#include "encodingWindows_test.hpp"
#include "encodingWindowsHelper.hpp"
#include <nlohmann/json.hpp>

#include <string>
#include <windows.h>

void EncodingWindowsHelperTest::SetUp() {};

void EncodingWindowsHelperTest::TearDown() {};

TEST_F(EncodingWindowsHelperTest, NoExceptConversion)
{
    nlohmann::json test;
    std::wstring wideString = L"Eines de correcció del Microsoft Office 2016: català";
    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wideString.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string multibyteString(bufferSize, 0);
    WideCharToMultiByte(CP_UTF8, 0, wideString.c_str(), -1, &multibyteString[0], bufferSize, nullptr, nullptr);
    test["correct"] = Utils::stringAnsiToStringUTF8(multibyteString);
    EXPECT_NO_THROW(test.dump());
}

TEST_F(EncodingWindowsHelperTest, ReturnValueEmptyConversion)
{
    EXPECT_EQ(Utils::stringAnsiToStringUTF8(""), "");
}
