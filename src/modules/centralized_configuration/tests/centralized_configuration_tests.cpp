#include <gtest/gtest.h>

#include <centralized_configuration.hpp>

using centralized_configuration::CentralizedConfiguration;

TEST(CentralizedConfiguration, Constructor)
{
    EXPECT_NO_THROW(
        [[maybe_unused]] CentralizedConfiguration centralizedConfiguration
    );
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
