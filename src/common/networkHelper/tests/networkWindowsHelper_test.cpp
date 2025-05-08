#include "networkWindowsHelper_test.hpp"
#include "networkWindowsHelper.hpp"

#include <array>

void NetworkWindowsHelperTest::SetUp()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
};

void NetworkWindowsHelperTest::TearDown()
{
    WSACleanup();
};

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

TEST_F(NetworkWindowsHelperTest, getIpV6AddressNullInputReturnsEmptyString)
{
    EXPECT_EQ(Utils::getIpV6Address(nullptr), "");
}

TEST_F(NetworkWindowsHelperTest, getIpV6AddressAllZerosReturnsDoubleColon)
{
    std::array<uint8_t, 16> addr = {0};
    EXPECT_EQ(Utils::getIpV6Address(addr.data()), "::");
}

TEST_F(NetworkWindowsHelperTest, getIpV6AddressLoopbackReturnsDoubleColonOne)
{
    // ::1
    std::array<uint8_t, 16> addr = {0};
    addr[15] = 1;
    EXPECT_EQ(Utils::getIpV6Address(addr.data()), "::1");
}

TEST_F(NetworkWindowsHelperTest, getIpV6AddressValidAddressReturnsCompressedIPv6)
{
    std::array<uint8_t, 16> addr = {
        0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x47, 0xf9, 0x90, 0xce, 0xdd, 0xf9, 0x71, 0x1a};
    EXPECT_EQ(Utils::getIpV6Address(addr.data()), "fe80::47f9:90ce:ddf9:711a");
}

TEST_F(NetworkWindowsHelperTest, getIpV6AddressGlobalUnicastReturnsCorrectFormat)
{
    std::array<uint8_t, 16> addr = {
        0x20, 0x01, 0x0d, 0xb8, 0x85, 0xa3, 0x00, 0x00, 0x00, 0x00, 0x8a, 0x2e, 0x03, 0x70, 0x73, 0x34};
    EXPECT_EQ(Utils::getIpV6Address(addr.data()), "2001:db8:85a3::8a2e:370:7334");
}

TEST_F(NetworkWindowsHelperTest, getIpV6AddressLinkLocalReturnsCorrectFormat)
{
    std::array<uint8_t, 16> addr = {
        0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xfe, 0x23, 0x45, 0x67, 0x89, 0x0a, 0x00, 0x00};
    EXPECT_EQ(Utils::getIpV6Address(addr.data()), "fe80::1ff:fe23:4567:890a:0");
}

TEST_F(NetworkWindowsHelperTest, getIpV6AddressMulticastReturnsCorrectFormat)
{
    std::array<uint8_t, 16> addr = {
        0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
    EXPECT_EQ(Utils::getIpV6Address(addr.data()), "ff02::1");
}

TEST_F(NetworkWindowsHelperTest, getIpV6AddressMultipleZeroBlocksCompressesCorrectly)
{
    std::array<uint8_t, 16> addr = {
        0x20, 0x01, 0x0d, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x42, 0x83, 0x29};
    EXPECT_EQ(Utils::getIpV6Address(addr.data()), "2001:db8::ff00:42:8329");
}

TEST_F(NetworkWindowsHelperTest, getIpV6AddressLeadingZerosInHextetsOmitsLeadingZeros)
{
    std::array<uint8_t, 16> addr = {
        0x20, 0x01, 0x0d, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x42, 0x83, 0x29};
    EXPECT_EQ(Utils::getIpV6Address(addr.data()), "2001:db8::ff00:42:8329");
}

TEST_F(NetworkWindowsHelperTest, getIpV6AddressIPv4MappedReturnsCorrectFormat)
{
    std::array<uint8_t, 16> addr = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xc0, 0xa8, 0x01, 0x01};
    EXPECT_EQ(Utils::getIpV6Address(addr.data()), "::ffff:192.168.1.1");
}

TEST_F(NetworkWindowsHelperTest, getIpV6AddressNoZeroBlocksReturnsFullAddress)
{
    std::array<uint8_t, 16> addr = {
        0x20, 0x01, 0x0d, 0xb8, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x12, 0x34, 0x56, 0x78};
    EXPECT_EQ(Utils::getIpV6Address(addr.data()), "2001:db8:1234:5678:9abc:def0:1234:5678");
}

TEST_F(NetworkWindowsHelperTest, getIpV6AddressEndWithZerosCompressesCorrectly)
{
    std::array<uint8_t, 16> addr = {
        0x20, 0x01, 0x0d, 0xb8, 0x12, 0x34, 0x56, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    EXPECT_EQ(Utils::getIpV6Address(addr.data()), "2001:db8:1234:5678::");
}

TEST_F(NetworkWindowsHelperTest, getIpV6AddressStartWithZerosCompressesCorrectly)
{
    std::array<uint8_t, 16> addr = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x00, 0x00};
    EXPECT_EQ(Utils::getIpV6Address(addr.data()), "::1234:5678:9abc:def0:0");
}

TEST_F(NetworkWindowsHelperTest, getIpV6AddressMiddleZerosCompressesCorrectly)
{
    std::array<uint8_t, 16> addr = {
        0x20, 0x01, 0x0d, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78, 0x00, 0x00};
    EXPECT_EQ(Utils::getIpV6Address(addr.data()), "2001:db8::1234:5678:0");
}

TEST_F(NetworkWindowsHelperTest, getIpV6AddressAllOnesReturnsCorrectFormat)
{
    std::array<uint8_t, 16> addr = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    EXPECT_EQ(Utils::getIpV6Address(addr.data()), "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff");
}

TEST_F(NetworkWindowsHelperTest, getIpV6AddressCompressionAtStartAndEndCompressesCorrectly)
{
    std::array<uint8_t, 16> addr = {
        0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x00, 0x03, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    EXPECT_EQ(Utils::getIpV6Address(addr.data()), "0:1:2:3:4::");
}

TEST_F(NetworkWindowsHelperTest, getIpV6AddressNullPointerReturnsEmptyString)
{
    EXPECT_EQ(Utils::getIpV6Address(nullptr), "");
}

TEST_F(NetworkWindowsHelperTest, getIpV6AddressNullBufferWithSizeReturnsEmptyString)
{
    uint8_t* nullBuffer = nullptr;
    EXPECT_EQ(Utils::getIpV6Address(nullBuffer), "");
}

TEST_F(NetworkWindowsHelperTest, getIpV6AddressZeroLengthBufferReturnsEmptyString)
{
    std::array<uint8_t, 0> emptyAddr;
    EXPECT_EQ(Utils::getIpV6Address(emptyAddr.data()), "");
}
