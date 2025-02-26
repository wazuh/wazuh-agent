#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <windows_service.hpp>

#include <iwindows_api_facade.hpp>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

class MockWindowsApiFacade : public windows_api_facade::IWindowsApiFacade
{
public:
    MOCK_METHOD(void*, OpenSCM, (unsigned int desiredAccess), (const override));
    MOCK_METHOD(void*,
                OpenSvc,
                (void* serviceHandle, const std::string& serviceName, unsigned int desiredAccess),
                (const override));
    MOCK_METHOD(void*,
                CreateSvc,
                (void* serviceHandle, const std::string& serviceName, const std::string& exePath),
                (const override));
    MOCK_METHOD(bool, DeleteSvc, (void* serviceHandle), (const override));
};

class WindowsServiceTest : public ::testing::Test
{
protected:
    MockWindowsApiFacade mockWindowsApiFacade;

    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(WindowsServiceTest, InstallService_FailOpenSCM)
{
    EXPECT_CALL(mockWindowsApiFacade, OpenSCM(SC_MANAGER_CREATE_SERVICE)).WillOnce(testing::Return(nullptr));

    bool result = windows_service::InstallService(mockWindowsApiFacade);

    EXPECT_FALSE(result);
}

TEST_F(WindowsServiceTest, InstallService_FailCreateSvc)
{
    EXPECT_CALL(mockWindowsApiFacade, OpenSCM(SC_MANAGER_CREATE_SERVICE))
        .WillOnce(testing::Return(reinterpret_cast<SC_HANDLE>(1)));
    EXPECT_CALL(mockWindowsApiFacade, CreateSvc(testing::_, testing::_, testing::_)).WillOnce(testing::Return(nullptr));

    bool result = windows_service::InstallService(mockWindowsApiFacade);

    EXPECT_FALSE(result);
}

TEST_F(WindowsServiceTest, InstallService_Success)
{
    EXPECT_CALL(mockWindowsApiFacade, OpenSCM(SC_MANAGER_CREATE_SERVICE))
        .WillOnce(testing::Return(reinterpret_cast<SC_HANDLE>(1)));
    EXPECT_CALL(mockWindowsApiFacade, CreateSvc(testing::_, "Wazuh Agent", testing::_))
        .WillOnce(testing::Return(reinterpret_cast<SC_HANDLE>(1)));

    bool result = windows_service::InstallService(mockWindowsApiFacade);

    EXPECT_TRUE(result);
}

TEST_F(WindowsServiceTest, RemoveService_FailOpenSCM)
{
    EXPECT_CALL(mockWindowsApiFacade, OpenSCM(SC_MANAGER_CREATE_SERVICE)).WillOnce(testing::Return(nullptr));

    bool result = windows_service::RemoveService(mockWindowsApiFacade);

    EXPECT_FALSE(result);
}

TEST_F(WindowsServiceTest, RemoveService_FailOpenSvc)
{
    EXPECT_CALL(mockWindowsApiFacade, OpenSCM(SC_MANAGER_CREATE_SERVICE))
        .WillOnce(testing::Return(reinterpret_cast<SC_HANDLE>(1)));
    EXPECT_CALL(mockWindowsApiFacade, OpenSvc(testing::_, testing::_, DELETE)).WillOnce(testing::Return(nullptr));

    bool result = windows_service::RemoveService(mockWindowsApiFacade);

    EXPECT_FALSE(result);
}

TEST_F(WindowsServiceTest, RemoveService_FailDeleteSvc)
{
    EXPECT_CALL(mockWindowsApiFacade, OpenSCM(SC_MANAGER_CREATE_SERVICE))
        .WillOnce(testing::Return(reinterpret_cast<SC_HANDLE>(1)));
    EXPECT_CALL(mockWindowsApiFacade, OpenSvc(testing::_, testing::_, DELETE))
        .WillOnce(testing::Return(reinterpret_cast<SC_HANDLE>(1)));
    EXPECT_CALL(mockWindowsApiFacade, DeleteSvc(reinterpret_cast<SC_HANDLE>(1))).WillOnce(testing::Return(false));

    bool result = windows_service::RemoveService(mockWindowsApiFacade);

    EXPECT_FALSE(result);
}

TEST_F(WindowsServiceTest, RemoveService_Success)
{
    EXPECT_CALL(mockWindowsApiFacade, OpenSCM(SC_MANAGER_CREATE_SERVICE))
        .WillOnce(testing::Return(reinterpret_cast<SC_HANDLE>(1)));
    EXPECT_CALL(mockWindowsApiFacade, OpenSvc(testing::_, testing::_, DELETE))
        .WillOnce(testing::Return(reinterpret_cast<SC_HANDLE>(1)));
    EXPECT_CALL(mockWindowsApiFacade, DeleteSvc(reinterpret_cast<SC_HANDLE>(1))).WillOnce(testing::Return(true));

    bool result = windows_service::RemoveService(mockWindowsApiFacade);

    EXPECT_TRUE(result);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
