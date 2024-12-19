#include <http_socket.hpp>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <iosfwd>
#include <thread>

class TestableHttpSocket : public http_client::HttpSocket
{
public:
    explicit TestableHttpSocket(const boost::asio::any_io_executor& ioContext)
        : HttpSocket(ioContext)
    {
    }

    void CallAsyncConnect(const boost::asio::ip::tcp::resolver::results_type& endpoints,
                          boost::system::error_code& ec,
                          bool& connectionSuccess) override
    {
        std::cout << "Delay before connect: " << m_nDelay << "\n";
        if (m_nDelay)
        {
            auto timer = std::make_shared<boost::asio::steady_timer>(m_socket.get_executor());
            timer->expires_after(std::chrono::seconds(m_nDelay));
            timer->async_wait(
                [this, &ec, &connectionSuccess, &endpoints](const boost::system::error_code& timerError)
                {
                    if (!timerError)
                    {
                        std::cout << "Executing CallAsyncConnect\n";
                        HttpSocket::CallAsyncConnect(endpoints, ec, connectionSuccess);
                        std::cout << "CallAsyncConnect returned " << ec.message() << "\n";
                    }
                });
        }
        else
        {
            HttpSocket::CallAsyncConnect(endpoints, ec, connectionSuccess);
        }
    }

    long m_nDelay {};
};

// Test fixture
class HttpSocketTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_socket = std::make_unique<TestableHttpSocket>(m_ioContext.get_executor());
    }

    void TearDown() override {}

    std::unique_ptr<TestableHttpSocket> m_socket;
    boost::asio::io_context m_ioContext;

    // Helper method to create sample endpoints
    boost::asio::ip::tcp::resolver::results_type CreateMockEndpoints()
    {
        boost::asio::ip::tcp::resolver resolver(m_ioContext.get_executor());
        // return resolver.resolve("192.168.0.156", "55000");
        return resolver.resolve("example.com", "80");
    }
};

// Test Connect method
TEST_F(HttpSocketTest, ConnectSuccessful)
{
    auto endpoints = std::make_shared<boost::asio::ip::tcp::resolver::results_type>(CreateMockEndpoints());
    auto ec = std::make_shared<boost::system::error_code>();

    std::cout << "Attempting to connect\n";

    m_socket->m_nDelay = 0;
    m_socket->Connect(m_ioContext, *endpoints, *ec, std::chrono::seconds(1));

    std::cout << "Connection returned: " << ec->message() << " (" << ec->value() << ")\n";

    EXPECT_EQ(ec->value(), 0) << "Connection should succeed";
}

TEST_F(HttpSocketTest, ConnectTimeout)
{
    auto endpoints = std::make_shared<boost::asio::ip::tcp::resolver::results_type>(CreateMockEndpoints());
    auto ec = std::make_shared<boost::system::error_code>();

    std::cout << "Attempting to connect\n";

    m_socket->m_nDelay = 3;
    m_socket->Connect(m_ioContext, *endpoints, *ec, std::chrono::seconds(1));

    std::cout << "Connection returned: " << ec->message() << " (" << ec->value() << ")\n";

    EXPECT_NE(ec->value(), 0) << "Connection should fail";
}

// Main function
int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
