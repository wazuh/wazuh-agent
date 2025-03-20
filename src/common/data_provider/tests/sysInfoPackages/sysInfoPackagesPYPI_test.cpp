#include "sysInfoPackagesPYPI_test.hpp"

using testing::_; // NOLINT(bugprone-reserved-identifier)
using testing::Return;
using testing::ReturnRef;

TEST_F(PYPITest, getPackagesTest)
{
    const std::vector<std::filesystem::path> fakeFiles = {"/fake/dir/file1", "/fake/dir/file2", "/fake/dir/file3"};

    EXPECT_CALL(*fileSystemWrapper, exists(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fileSystemWrapper, is_directory(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fileSystemWrapper, list_directory(_)).WillRepeatedly(Return(fakeFiles));
    EXPECT_CALL(*pypi, readLineByLine(_, _)).WillRepeatedly(Return());

    nlohmann::json capturedJson;
    auto callback = [&](nlohmann::json& j)
    {
        capturedJson = j;
    };

    const std::set<std::string> folders = {"/usr/local/lib/python3.9/site-packages"};

    EXPECT_CALL(*mockFileSystemUtils, expand_absolute_path(_, _))
        .WillRepeatedly([](const std::string& base, std::deque<std::string>& out) { out.push_back(base); });

    pypi->getPackages(folders, callback);

    EXPECT_TRUE(capturedJson.empty());
}

TEST_F(PYPITest, getPackages_NoFilesInDirectoryTest)
{
    const std::vector<std::filesystem::path> fakeFiles = {};

    EXPECT_CALL(*fileSystemWrapper, exists(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fileSystemWrapper, is_directory(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fileSystemWrapper, list_directory(_)).WillRepeatedly(Return(fakeFiles));

    nlohmann::json capturedJson;
    auto callback = [&](nlohmann::json& j)
    {
        capturedJson = j;
    };

    const std::set<std::string> folders = {"/usr/local/lib/python3.9/site-packages"};

    EXPECT_CALL(*mockFileSystemUtils, expand_absolute_path(_, _))
        .WillRepeatedly([](const std::string& base, std::deque<std::string>& out) { out.push_back(base); });

    pypi->getPackages(folders, callback);

    EXPECT_TRUE(capturedJson.empty());
}

TEST_F(PYPITest, getPackages_NonDirectoryPathTest)
{
    EXPECT_CALL(*fileSystemWrapper, exists(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fileSystemWrapper, is_directory(_)).WillRepeatedly(Return(false));

    const std::set<std::string> folders = {"/usr/local/lib/python3.9/site-packages"};
    nlohmann::json capturedJson;
    auto callback = [&](nlohmann::json& j)
    {
        capturedJson = j;
    };

    EXPECT_CALL(*mockFileSystemUtils, expand_absolute_path(_, _))
        .WillRepeatedly([](const std::string& base, std::deque<std::string>& out) { out.push_back(base); });

    pypi->getPackages(folders, callback);

    EXPECT_TRUE(capturedJson.empty());
}

TEST_F(PYPITest, getPackages_OneValidPackageTestEggInfo)
{
    const std::vector<std::filesystem::path> fakeFiles = {"/fake/dir/egg-info"};

    EXPECT_CALL(*fileSystemWrapper, exists(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fileSystemWrapper, is_directory(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fileSystemWrapper, list_directory(_)).WillRepeatedly(Return(fakeFiles));
    EXPECT_CALL(*fileSystemWrapper, is_regular_file(_)).WillRepeatedly(Return(true));

    std::vector<std::string> fakePackageLines = {"Name: TestPackage", "Version: 1.0.0"};
    EXPECT_CALL(*pypi, readLineByLine(std::filesystem::path("/fake/dir/egg-info"), _))
        .WillOnce(
            [&](const std::filesystem::path&, const std::function<bool(const std::string&)>& callback)
            {
                for (const auto& line : fakePackageLines)
                {
                    callback(line);
                }
            });

    nlohmann::json capturedJson;
    auto callback = [&](nlohmann::json& j)
    {
        capturedJson = j;
    };

    const std::set<std::string> folders = {"/usr/local/lib/python3.9/site-packages"};

    EXPECT_CALL(*mockFileSystemUtils, expand_absolute_path(_, _))
        .WillRepeatedly([](const std::string& base, std::deque<std::string>& out) { out.push_back(base); });

    pypi->getPackages(folders, callback);

    EXPECT_EQ(capturedJson.at("name"), "TestPackage");
    EXPECT_EQ(capturedJson.at("version"), "1.0.0");
}

TEST_F(PYPITest, getPackages_OneValidPackageTestNoRegularFileDistInfo)
{
    const std::vector<std::filesystem::path> fakeFiles = {"/fake/dir/dist-info"};

    EXPECT_CALL(*fileSystemWrapper, exists(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fileSystemWrapper, is_directory(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fileSystemWrapper, list_directory(_)).WillRepeatedly(Return(fakeFiles));
    EXPECT_CALL(*fileSystemWrapper, is_regular_file(_)).WillRepeatedly(Return(false));

    std::vector<std::string> fakePackageLines = {"Name: TestPackage", "Version: 1.0.0"};
    EXPECT_CALL(*pypi, readLineByLine(std::filesystem::path("/fake/dir/dist-info/METADATA"), _))
        .WillOnce(
            [&](const std::filesystem::path&, const std::function<bool(const std::string&)>& callback)
            {
                for (const auto& line : fakePackageLines)
                {
                    callback(line);
                }
            });

    nlohmann::json capturedJson;
    auto callback = [&](nlohmann::json& j)
    {
        capturedJson = j;
    };

    const std::set<std::string> folders = {"/usr/local/lib/python3.9/site-packages"};

    EXPECT_CALL(*mockFileSystemUtils, expand_absolute_path(_, _))
        .WillRepeatedly([](const std::string& base, std::deque<std::string>& out) { out.push_back(base); });

    pypi->getPackages(folders, callback);

    EXPECT_EQ(capturedJson.at("name"), "TestPackage");
    EXPECT_EQ(capturedJson.at("version"), "1.0.0");
}

TEST_F(PYPITest, getPackages_OneValidPackageTestDistInfo)
{
    const std::vector<std::filesystem::path> fakeFiles = {"/fake/dir/dist-info"};

    EXPECT_CALL(*fileSystemWrapper, exists(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fileSystemWrapper, is_directory(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fileSystemWrapper, list_directory(_)).WillRepeatedly(Return(fakeFiles));
    EXPECT_CALL(*fileSystemWrapper, is_regular_file(_)).WillRepeatedly(Return(true));

    std::vector<std::string> fakePackageLines = {"Name: TestPackage", "Version: 1.0.0"};
    EXPECT_CALL(*pypi, readLineByLine(std::filesystem::path("/fake/dir/dist-info"), _))
        .WillOnce(
            [&](const std::filesystem::path&, const std::function<bool(const std::string&)>& callback)
            {
                for (const auto& line : fakePackageLines)
                {
                    callback(line);
                }
            });

    nlohmann::json capturedJson;
    auto callback = [&](nlohmann::json& j)
    {
        capturedJson = j;
    };

    const std::set<std::string> folders = {"/usr/local/lib/python3.9/site-packages"};

    EXPECT_CALL(*mockFileSystemUtils, expand_absolute_path(_, _))
        .WillRepeatedly([](const std::string& base, std::deque<std::string>& out) { out.push_back(base); });

    pypi->getPackages(folders, callback);

    EXPECT_EQ(capturedJson.at("name"), "TestPackage");
    EXPECT_EQ(capturedJson.at("version"), "1.0.0");
}

TEST_F(PYPITest, getPackages_OneValidPackageTestNoRegularFileEggInfo)
{
    const std::vector<std::filesystem::path> fakeFiles = {"/fake/dir/egg-info"};

    EXPECT_CALL(*fileSystemWrapper, exists(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fileSystemWrapper, is_directory(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fileSystemWrapper, list_directory(_)).WillRepeatedly(Return(fakeFiles));
    EXPECT_CALL(*fileSystemWrapper, is_regular_file(_)).WillRepeatedly(Return(false));

    std::vector<std::string> fakePackageLines = {"Name: TestPackage", "Version: 1.0.0"};
    EXPECT_CALL(*pypi, readLineByLine(std::filesystem::path("/fake/dir/egg-info/PKG-INFO"), _))
        .WillOnce(
            [&](const std::filesystem::path&, const std::function<bool(const std::string&)>& callback)
            {
                for (const auto& line : fakePackageLines)
                {
                    callback(line);
                }
            });

    nlohmann::json capturedJson;
    auto callback = [&](nlohmann::json& j)
    {
        capturedJson = j;
    };

    const std::set<std::string> folders = {"/usr/local/lib/python3.9/site-packages"};

    EXPECT_CALL(*mockFileSystemUtils, expand_absolute_path(_, _))
        .WillRepeatedly([](const std::string& base, std::deque<std::string>& out) { out.push_back(base); });

    pypi->getPackages(folders, callback);

    EXPECT_EQ(capturedJson.at("name"), "TestPackage");
    EXPECT_EQ(capturedJson.at("version"), "1.0.0");
}

TEST_F(PYPITest, getPackages_MultipleValidPackagesTest)
{
    const std::vector<std::filesystem::path> fakeFiles = {"/fake/dir1/egg-info", "/fake/dir2/dist-info"};

    EXPECT_CALL(*fileSystemWrapper, exists(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fileSystemWrapper, is_directory(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fileSystemWrapper, list_directory(_)).WillRepeatedly(Return(fakeFiles));
    EXPECT_CALL(*fileSystemWrapper, is_regular_file(_)).WillRepeatedly(Return(true));

    std::vector<std::string> fakePackageLines1 = {"Name: TestPackage1", "Version: 1.0.0"};
    std::vector<std::string> fakePackageLines2 = {"Name: TestPackage2", "Version: 2.0.0"};

    EXPECT_CALL(*pypi, readLineByLine(std::filesystem::path("/fake/dir1/egg-info"), _))
        .WillOnce(
            [&](const std::filesystem::path&, const std::function<bool(const std::string&)>& callback)
            {
                for (const auto& line : fakePackageLines1)
                {
                    callback(line);
                }
            });

    EXPECT_CALL(*pypi, readLineByLine(std::filesystem::path("/fake/dir2/dist-info"), _))
        .WillOnce(
            [&](const std::filesystem::path&, const std::function<bool(const std::string&)>& callback)
            {
                for (const auto& line : fakePackageLines2)
                {
                    callback(line);
                }
            });

    bool foundPackage1 = false;
    bool foundPackage2 = false;

    const nlohmann::json capturedJson;
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

    const std::set<std::string> folders = {"/usr/local/lib/python3.9/site-packages"};

    EXPECT_CALL(*mockFileSystemUtils, expand_absolute_path(_, _))
        .WillRepeatedly([](const std::string& base, std::deque<std::string>& out) { out.push_back(base); });

    pypi->getPackages(folders, callback);

    EXPECT_TRUE(foundPackage1);
    EXPECT_TRUE(foundPackage2);
}

TEST_F(PYPITest, getPackages_InvalidPackageTest_NoLines)
{
    const std::vector<std::filesystem::path> fakeFiles = {"/fake/dir/egg-info"};

    EXPECT_CALL(*fileSystemWrapper, exists(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fileSystemWrapper, is_directory(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fileSystemWrapper, list_directory(_)).WillRepeatedly(Return(fakeFiles));
    EXPECT_CALL(*fileSystemWrapper, is_regular_file(_)).WillRepeatedly(Return(true));

    std::vector<std::string> fakePackageLines = {};

    EXPECT_CALL(*pypi, readLineByLine(std::filesystem::path("/fake/dir/egg-info"), _))
        .WillOnce(
            [&](const std::filesystem::path&, const std::function<bool(const std::string&)>& callback)
            {
                for (const auto& line : fakePackageLines)
                {
                    callback(line);
                }
            });

    nlohmann::json capturedJson;
    auto callback = [&](nlohmann::json& j)
    {
        capturedJson = j;
    };

    const std::set<std::string> folders = {"/usr/local/lib/python3.9/site-packages"};

    EXPECT_CALL(*mockFileSystemUtils, expand_absolute_path(_, _))
        .WillRepeatedly([](const std::string& base, std::deque<std::string>& out) { out.push_back(base); });

    pypi->getPackages(folders, callback);

    EXPECT_TRUE(capturedJson.empty());
}

TEST_F(PYPITest, getPackages_InvalidPackageTest_InvalidLines)
{
    const std::vector<std::filesystem::path> fakeFiles = {"/fake/dir/dist-info"};

    EXPECT_CALL(*fileSystemWrapper, exists(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fileSystemWrapper, is_directory(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fileSystemWrapper, list_directory(_)).WillRepeatedly(Return(fakeFiles));
    EXPECT_CALL(*fileSystemWrapper, is_regular_file(_)).WillRepeatedly(Return(true));

    std::vector<std::string> fakePackageLines = {"Invalid: TestPackage", "Invalid: 1.0.0"};

    EXPECT_CALL(*pypi, readLineByLine(std::filesystem::path("/fake/dir/dist-info"), _))
        .WillOnce(
            [&](const std::filesystem::path&, const std::function<bool(const std::string&)>& callback)
            {
                for (const auto& line : fakePackageLines)
                {
                    callback(line);
                }
            });

    nlohmann::json capturedJson;
    auto callback = [&](nlohmann::json& j)
    {
        capturedJson = j;
    };

    const std::set<std::string> folders = {"/usr/local/lib/python3.9/site-packages"};

    EXPECT_CALL(*mockFileSystemUtils, expand_absolute_path(_, _))
        .WillRepeatedly([](const std::string& base, std::deque<std::string>& out) { out.push_back(base); });

    pypi->getPackages(folders, callback);

    EXPECT_TRUE(capturedJson.empty());
}

TEST_F(PYPITest, getPackages_InvalidPackageTest_MissingName)
{
    const std::vector<std::filesystem::path> fakeFiles = {"/fake/dir/dist-info"};

    EXPECT_CALL(*fileSystemWrapper, exists(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fileSystemWrapper, is_directory(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fileSystemWrapper, list_directory(_)).WillRepeatedly(Return(fakeFiles));
    EXPECT_CALL(*fileSystemWrapper, is_regular_file(_)).WillRepeatedly(Return(true));

    std::vector<std::string> fakePackageLines = {"Version: 1.0.0"};

    EXPECT_CALL(*pypi, readLineByLine(std::filesystem::path("/fake/dir/dist-info"), _))
        .WillOnce(
            [&](const std::filesystem::path&, const std::function<bool(const std::string&)>& callback)
            {
                for (const auto& line : fakePackageLines)
                {
                    callback(line);
                }
            });

    nlohmann::json capturedJson;
    auto callback = [&](nlohmann::json& j)
    {
        capturedJson = j;
    };

    const std::set<std::string> folders = {"/usr/local/lib/python3.9/site-packages"};

    EXPECT_CALL(*mockFileSystemUtils, expand_absolute_path(_, _))
        .WillRepeatedly([](const std::string& base, std::deque<std::string>& out) { out.push_back(base); });

    pypi->getPackages(folders, callback);

    EXPECT_TRUE(capturedJson.empty());
}

TEST_F(PYPITest, getPackages_InvalidPackageTest_MissingVersion)
{
    const std::vector<std::filesystem::path> fakeFiles = {"/fake/dir/dist-info"};

    EXPECT_CALL(*fileSystemWrapper, exists(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fileSystemWrapper, is_directory(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fileSystemWrapper, list_directory(_)).WillRepeatedly(Return(fakeFiles));
    EXPECT_CALL(*fileSystemWrapper, is_regular_file(_)).WillRepeatedly(Return(true));

    std::vector<std::string> fakePackageLines = {"Name: TestPackage"};

    EXPECT_CALL(*pypi, readLineByLine(std::filesystem::path("/fake/dir/dist-info"), _))
        .WillOnce(
            [&](const std::filesystem::path&, const std::function<bool(const std::string&)>& callback)
            {
                for (const auto& line : fakePackageLines)
                {
                    callback(line);
                }
            });

    nlohmann::json capturedJson;
    auto callback = [&](nlohmann::json& j)
    {
        capturedJson = j;
    };

    const std::set<std::string> folders = {"/usr/local/lib/python3.9/site-packages"};

    EXPECT_CALL(*mockFileSystemUtils, expand_absolute_path(_, _))
        .WillRepeatedly([](const std::string& base, std::deque<std::string>& out) { out.push_back(base); });

    pypi->getPackages(folders, callback);

    std::cout << capturedJson.dump(4) << '\n';
    EXPECT_TRUE(capturedJson.empty());
}
