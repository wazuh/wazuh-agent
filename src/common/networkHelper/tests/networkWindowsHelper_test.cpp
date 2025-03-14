#include "networkWindowsHelper_test.hpp"
#include "networkWindowsHelper.hpp"

void NetworkWindowsHelperTest::SetUp() {};

void NetworkWindowsHelperTest::TearDown() {};

TEST_F(NetworkWindowsHelperTest, ipv6NetMask_64)
{
    const int addressPrefixLength {64};
    const std::string expectedNetMask {"ffff:ffff:ffff:ffff::"};
    std::string netMask {Utils::ipv6Netmask(addressPrefixLength)};
    EXPECT_EQ(expectedNetMask, netMask);
}

TEST_F(NetworkWindowsHelperTest, ipv6NetMask_127)
{
    const int addressPrefixLength {127};
    const std::string expectedNetMask {"ffff:ffff:ffff:ffff:ffff:ffff:ffff:fffe"};
    std::string netMask {Utils::ipv6Netmask(addressPrefixLength)};
    EXPECT_EQ(expectedNetMask, netMask);
}

TEST_F(NetworkWindowsHelperTest, ipv6NetMask_55)
{
    const int addressPrefixLength {55};
    const std::string expectedNetMask {"ffff:ffff:ffff:fe::"};
    std::string netMask {Utils::ipv6Netmask(addressPrefixLength)};
    EXPECT_EQ(expectedNetMask, netMask);
}

TEST_F(NetworkWindowsHelperTest, ipv6NetMask_77)
{
    const int addressPrefixLength {77};
    const std::string expectedNetMask {"ffff:ffff:ffff:ffff:fff8::"};
    std::string netMask {Utils::ipv6Netmask(addressPrefixLength)};
    EXPECT_EQ(expectedNetMask, netMask);
}

TEST_F(NetworkWindowsHelperTest, ipv6NetMask_72)
{
    const int addressPrefixLength {72};
    const std::string expectedNetMask {"ffff:ffff:ffff:ffff:ff00::"};
    std::string netMask {Utils::ipv6Netmask(addressPrefixLength)};
    EXPECT_EQ(expectedNetMask, netMask);
}

TEST_F(NetworkWindowsHelperTest, ipv6NetMask_INVALID)
{
    const int addressPrefixLength {130};
    std::string netMask {Utils::ipv6Netmask(addressPrefixLength)};
    EXPECT_TRUE(netMask.empty());
}
