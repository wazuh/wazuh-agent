#include "sysInfoPackagesNPM_test.hpp"

using testing::_; // NOLINT(bugprone-reserved-identifier)
using testing::Return;
using testing::ReturnRef;

TEST_F(NPMTest, getPackages_ValidPackagesTest)
{
    const std::vector<std::filesystem::path> fakePackages = {"/fake/node_modules/package1",
                                                             "/fake/node_modules/package2"};

    EXPECT_CALL(*fileSystemWrapper, exists(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fileSystemWrapper, is_directory(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fileSystemWrapper, list_directory(_)).WillRepeatedly(Return((fakePackages)));

    const nlohmann::json fakePackageJson1 = {{"name", "TestPackage1"}, {"version", "1.0.0"}};
    const nlohmann::json fakePackageJson2 = {{"name", "TestPackage2"}, {"version", "2.0.0"}};

    EXPECT_CALL(*npm, readJson(std::filesystem::path("/fake/node_modules/package1/package.json")))
        .WillOnce(Return(fakePackageJson1));
    EXPECT_CALL(*npm, readJson(std::filesystem::path("/fake/node_modules/package2/package.json")))
        .WillOnce(Return(fakePackageJson2));

    bool foundPackage1 = false;
    bool foundPackage2 = false;

    const std::set<std::string> folders = {"/fake"};

    EXPECT_CALL(*mockFileSystemUtils, expand_absolute_path(_, _))
        .WillRepeatedly([](const std::string& base, std::deque<std::string>& out) { out.push_back(base); });

    auto callback = [&](nlohmann::json& json)
    {
        if (json.at("name") == "TestPackage1" && json.at("version") == "1.0.0")
        {
            foundPackage1 = true;
        }
        else if (json.at("name") == "TestPackage2" && json.at("version") == "2.0.0")
        {
            foundPackage2 = true;
        }
    };

    npm->getPackages(folders, callback);

    EXPECT_TRUE(foundPackage1);
    EXPECT_TRUE(foundPackage2);
}

TEST_F(NPMTest, getPackages_NoPackagesFoundTest)
{
    const std::vector<std::filesystem::path> fakePackages = {};

    EXPECT_CALL(*fileSystemWrapper, exists(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fileSystemWrapper, list_directory(_)).WillRepeatedly(Return(fakePackages));

    bool callbackCalled = false;

    const std::set<std::string> folders = {"/fake"};

    npm->getPackages(folders, [&](nlohmann::json&) { callbackCalled = true; });

    EXPECT_FALSE(callbackCalled);
}

TEST_F(NPMTest, getPackages_NoPackageJsonTest)
{
    const std::vector<std::filesystem::path> fakePackages = {"/fake/node_modules/package1"};

    EXPECT_CALL(*fileSystemWrapper, exists(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fileSystemWrapper, is_directory(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fileSystemWrapper, list_directory(_)).WillRepeatedly(Return(fakePackages));

    EXPECT_CALL(*npm, readJson(std::filesystem::path("/fake/node_modules/package1/package.json")))
        .WillOnce(Return(nlohmann::json()));

    bool callbackCalled = false;

    const std::set<std::string> folders = {"/fake"};

    EXPECT_CALL(*mockFileSystemUtils, expand_absolute_path(_, _))
        .WillRepeatedly([](const std::string& base, std::deque<std::string>& out) { out.push_back(base); });

    npm->getPackages(folders, [&](nlohmann::json&) { callbackCalled = true; });

    EXPECT_FALSE(callbackCalled);
}

TEST_F(NPMTest, getPackages_InvalidPackageJsonNameTest)
{
    const std::vector<std::filesystem::path> fakePackages = {"/fake/node_modules/package1"};

    EXPECT_CALL(*fileSystemWrapper, exists(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fileSystemWrapper, is_directory(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fileSystemWrapper, list_directory(_)).WillRepeatedly(Return(fakePackages));

    EXPECT_CALL(*npm, readJson(std::filesystem::path("/fake/node_modules/package1/package.json")))
        .WillOnce(Return(nlohmann::json::parse(R"({"name": 1})")));

    bool callbackCalled = false;

    const std::set<std::string> folders = {"/fake"};

    EXPECT_CALL(*mockFileSystemUtils, expand_absolute_path(_, _))
        .WillRepeatedly([](const std::string& base, std::deque<std::string>& out) { out.push_back(base); });

    npm->getPackages(folders, [&](nlohmann::json&) { callbackCalled = true; });

    EXPECT_FALSE(callbackCalled);
}

TEST_F(NPMTest, getPackages_InvalidPackageJsonVersionTest)
{
    const std::vector<std::filesystem::path> fakePackages = {"/fake/node_modules/package1"};

    EXPECT_CALL(*fileSystemWrapper, exists(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fileSystemWrapper, is_directory(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fileSystemWrapper, list_directory(_)).WillRepeatedly(Return(fakePackages));

    EXPECT_CALL(*npm, readJson(std::filesystem::path("/fake/node_modules/package1/package.json")))
        .WillOnce(Return(nlohmann::json::parse(R"({"name": "TestPackage1", "version": 1})")));

    bool callbackCalled = false;

    const std::set<std::string> folders = {"/fake"};

    EXPECT_CALL(*mockFileSystemUtils, expand_absolute_path(_, _))
        .WillRepeatedly([](const std::string& base, std::deque<std::string>& out) { out.push_back(base); });

    npm->getPackages(folders, [&](nlohmann::json&) { callbackCalled = true; });

    EXPECT_FALSE(callbackCalled);
}

TEST_F(NPMTest, getPackages_ValidPackageJson2Test)
{
    const std::vector<std::filesystem::path> fakePackages = {"/fake/node_modules/package1",
                                                             "/fake/node_modules/package2"};

    EXPECT_CALL(*fileSystemWrapper, exists(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fileSystemWrapper, is_directory(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fileSystemWrapper, list_directory(_)).WillOnce(Return(fakePackages));

    EXPECT_CALL(*npm, readJson(std::filesystem::path("/fake/node_modules/package1/package.json")))
        .WillOnce(Return(nlohmann::json::parse(R"({"name": "TestPackage1", "version": "1.0.0"})")));

    EXPECT_CALL(*npm, readJson(std::filesystem::path("/fake/node_modules/package2/package.json")))
        .WillOnce(Return(nlohmann::json::parse(R"({"name": "TestPackage2", "version": "1.0.0"})")));

    bool callbackCalledFirst = false;
    bool callbackCalledSecond = false;

    const std::set<std::string> folders = {"/fake"};

    EXPECT_CALL(*mockFileSystemUtils, expand_absolute_path(_, _))
        .WillRepeatedly([](const std::string& base, std::deque<std::string>& out) { out.push_back(base); });

    npm->getPackages(folders,
                     [&](nlohmann::json& j)
                     {
                         if (j.at("name") == "TestPackage1" && j.at("version") == "1.0.0")
                         {
                             callbackCalledFirst = true;
                         }
                         else if (j.at("name") == "TestPackage2" && j.at("version") == "1.0.0")
                         {
                             callbackCalledSecond = true;
                         }
                         else
                         {
                             FAIL();
                         }
                     });

    EXPECT_TRUE(callbackCalledFirst);
    EXPECT_TRUE(callbackCalledSecond);
}
