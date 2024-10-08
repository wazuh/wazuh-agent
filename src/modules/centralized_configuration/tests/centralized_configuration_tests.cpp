#include <gtest/gtest.h>

#include <centralized_configuration.hpp>

using centralized_configuration::CentralizedConfiguration;

TEST(CentralizedConfiguration, Constructor)
{
    EXPECT_NO_THROW(
        [[maybe_unused]] CentralizedConfiguration centralizedConfiguration
    );
}

TEST(CentralizedConfiguration, ImplementsModuleWrapperInterface)
{
    CentralizedConfiguration centralizedConfiguration;
    EXPECT_NO_THROW(centralizedConfiguration.Start());
    EXPECT_NO_THROW(centralizedConfiguration.Stop());
    EXPECT_NO_THROW(centralizedConfiguration.Name());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
